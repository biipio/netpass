#include "curl-handler.h"
#include "cecd.h"
#include "api.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#define MAX_CONNECTIONS 20

#define SOC_ALIGN 0x1000
#define SOC_BUFFERSIZE 0x100000
static u32 *SOC_buffer = NULL;

static Thread curl_multi_thread;
static bool running = false;
static u8 mac[6] = {0};

#define CURL_HANDLE_STATUS_FREE 0
#define CURL_HANDLE_STATUS_RESERVED 1
#define CURL_HANDLE_STATUS_PENDING 2
#define CURL_HANDLE_STATUS_RUNNING 3
#define CURL_HANDLE_STATUS_DONE 4
#define CURL_HANDLE_STATUS_RESET 5

struct CurlHandle {
	CURL* handle;
	CURLcode result;
	volatile int status;
	char* method;
	char* url;
	int size;
	u8* body;
	Result res;
	CurlReply reply;
};

static struct CurlHandle handles[MAX_CONNECTIONS];

Result getMac(u8 mac[6]) {
	Result res = 0;
	Handle handle;
	res = srvGetServiceHandle(&handle, "nwm::SOC");
	if (R_FAILED(res)) return res;

	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(8, 1, 0);
	cmdbuf[1] = 6;

	u32 saved_threadstorage[2];

	u32 *staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];
	staticbufs[0] = IPC_Desc_StaticBuffer(6, 0);
	staticbufs[1] = (u32)mac;
	res = svcSendSyncRequest(handle);
	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];
	if (R_FAILED(res)) return res;
	res = (Result)cmdbuf[1];
	memcpy(mac, (u8*)cmdbuf[3], 6);
	return res;
}

size_t curlWrite(void *data, size_t size, size_t nmemb, void* ptr) {
	CurlReply* r = (CurlReply*)ptr;
	if (!r) {
		return size*nmemb; // let's just pretend we did all correct
	}
	size_t new_len = r->len + size*nmemb;
	if (new_len > MAX_MESSAGE_SIZE) {
		return 0;
	}
	memcpy(r->ptr + r->len, data, size*nmemb);
	r->ptr[new_len] = '\0';
	r->len = new_len;
	return size*nmemb;
}

void curlFreeHandler(int offset) {
	handles[offset].status = CURL_HANDLE_STATUS_RESET;
}

Result httpRequest(char* method, char* url, int size, u8* body, CurlReply** reply) {
	Result res = 0;
	int curl_handle_slot = 0;
	bool found_handle_slot = false;
	for (; curl_handle_slot < MAX_CONNECTIONS; curl_handle_slot++) {
		if (handles[curl_handle_slot].status == CURL_HANDLE_STATUS_FREE) {
			handles[curl_handle_slot].status = CURL_HANDLE_STATUS_RESERVED;
			found_handle_slot = true;
			break;
		}
	}
	if (!found_handle_slot) {
		// TODO: dunno, wait or something?
		return -1;
	}

	handles[curl_handle_slot].method = method;
	handles[curl_handle_slot].url = url;
	handles[curl_handle_slot].size = size;
	handles[curl_handle_slot].body = body;
	handles[curl_handle_slot].status = CURL_HANDLE_STATUS_PENDING;
	handles[curl_handle_slot].reply.ptr = (u8*)reply;
	// request is being sent, let's wait until it is back
	
	while (handles[curl_handle_slot].status != CURL_HANDLE_STATUS_DONE) {
		//printf("%d", handles[curl_handle_slot].status);
		svcSleepThread((u64)1000000 * 100);
	}

	res = handles[curl_handle_slot].res;
	if (reply) {
		*reply = &handles[curl_handle_slot].reply;
	} else {
		curlFreeHandler(curl_handle_slot);
	}
	return res;
}

static CURLM* curl_multi_handle;

void curl_multi_loop_request_finish(int i) {
	struct CurlHandle* h = &handles[i];
	h->res = 0;
	h->res = h->result;
	if (h->res != CURLE_OK) {
		h->res = -h->res;
		goto cleanup;
	}
	long http_code = 0;
	curl_easy_getinfo(h->handle, CURLINFO_RESPONSE_CODE, &http_code);
	if (!(http_code >= 200 && http_code < 300)) {
		h->res = -http_code;
		goto cleanup;
	}
	h->res = http_code;
cleanup:
	curl_multi_remove_handle(curl_multi_handle, h->handle);
	curl_easy_cleanup(h->handle);
	h->handle = 0;
	h->status = CURL_HANDLE_STATUS_DONE;
}

void curl_multi_loop_request_setup(int i) {
	struct CurlHandle* h = &handles[i];
	h->handle = curl_easy_init();
	if (!h->handle) {
		h->res = -1;
		h->status = CURL_HANDLE_STATUS_DONE;
		return;
	}
	struct curl_slist* headers = NULL;

	// add mac header
	char header_mac[25];
	char* header_mac_i = header_mac + snprintf(header_mac, 25 - 12, "3ds-mac: ");
	for (int j = 0; j < 6; j++) {
		header_mac_i += sprintf(header_mac_i, "%02X", mac[j]);
	}
	headers = curl_slist_append(headers, header_mac);

	if (h->body) {
		curl_easy_setopt(h->handle, CURLOPT_POSTFIELDS, h->body);
		headers = curl_slist_append(headers, "Content-Type: application/binary");
		curl_easy_setopt(h->handle, CURLOPT_POSTFIELDSIZE, h->size);
	}

	// set some options
	curl_easy_setopt(h->handle, CURLOPT_URL, h->url);
	curl_easy_setopt(h->handle, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(h->handle, CURLOPT_USERAGENT, "3ds");
	curl_easy_setopt(h->handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(h->handle, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(h->handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(h->handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(h->handle, CURLOPT_CUSTOMREQUEST, h->method);
	curl_easy_setopt(h->handle, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(h->handle, CURLOPT_SERVER_RESPONSE_TIMEOUT, 10);
	curl_easy_setopt(h->handle, CURLOPT_CONNECTTIMEOUT, 20);
	curl_easy_setopt(h->handle, CURLOPT_NOSIGNAL, 0);
	curl_easy_setopt(h->handle, CURLOPT_SSL_VERIFYPEER, 1);
	curl_easy_setopt(h->handle, CURLOPT_CAINFO, "romfs:/certs.pem");

	curl_easy_setopt(h->handle, CURLOPT_WRITEFUNCTION, curlWrite);
	h->reply.len = 0;
	h->reply.offset = i;
	if (h->reply.ptr) {
		h->reply.ptr = malloc(MAX_MESSAGE_SIZE);
		if (!h->reply.ptr) {
			h->res = -1;
			h->status = CURL_HANDLE_STATUS_DONE;
			return;
		}
		curl_easy_setopt(h->handle, CURLOPT_WRITEDATA, &h->reply);
	} else {
		curl_easy_setopt(h->handle, CURLOPT_WRITEDATA, NULL);
	}
	h->status = CURL_HANDLE_STATUS_RUNNING;

	curl_multi_add_handle(curl_multi_handle, h->handle);
}

void curl_multi_loop(void* p) {
	curl_multi_handle = curl_multi_init();
	running = true;
	int openHandles = 0;
	do {
		CURLMcode mc = curl_multi_perform(curl_multi_handle, &openHandles);
		if (mc != CURLM_OK) {
			printf("ERROR curl multi fail: %u\n", mc);
			return;
		}
		CURLMsg* msg;
		int msgsLeft;
		while ((msg = curl_multi_info_read(curl_multi_handle, &msgsLeft))) {
			if (msg->msg == CURLMSG_DONE) {
				for (int i = 0; i < MAX_CONNECTIONS; i++) {
					if (handles[i].handle == msg->easy_handle) {
						handles[i].result = msg->data.result;
						curl_multi_loop_request_finish(i);
						break;
					}
				}
			}
		}
		if (!openHandles) {
			svcSleepThread((u64)1000000 * 100);
		} else {
			svcSleepThread(1000000);
		}
		for (int i = 0; i < MAX_CONNECTIONS; i++) {
			if (handles[i].status == CURL_HANDLE_STATUS_RESET) {
				if (handles[i].reply.ptr) {
					free(handles[i].reply.ptr);
				}
				handles[i].handle = 0;
				handles[i].result = 0;
				handles[i].status = CURL_HANDLE_STATUS_FREE;
			}
			if (handles[i].status == CURL_HANDLE_STATUS_PENDING) {
				curl_multi_loop_request_setup(i);
			}
		}
	} while (running);
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		if (handles[i].handle) {
			curl_multi_remove_handle(curl_multi_handle, handles[i].handle);
			curl_easy_cleanup(handles[i].handle);
		}
	}
	curl_multi_cleanup(curl_multi_handle);
}

Result curlInit(void) {
	Result res;
	// ok, we have to init this first
	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if (!SOC_buffer) return -1;
	res = socInit(SOC_buffer, SOC_BUFFERSIZE);
	if (R_FAILED(res)) return res;
	curl_global_init(CURL_GLOBAL_ALL);

	res = getMac(mac);
	if (R_FAILED(res)) return res;

	curl_multi_thread = threadCreate(curl_multi_loop, NULL, 8*1024, main_thread_prio()-1, -2, false);

	return res;
}

void curlExit(void) {
	running = false;
	//threadFree(curl_multi_thread);
	curl_global_cleanup();
	socExit();
}