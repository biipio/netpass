/**
 * NetPass
 * Copyright (C) 2024 Sorunome
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "switch.h"
#include "../report.h"
#include "../hmac_sha256/sha256.h"
#include "../render.h"
#include "../utils.h"
#include <stdlib.h>
#include <malloc.h>
#define N(x) scenes_report_list_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	C2D_TextBuf g_staticBuf;
	ReportList* list;
	C2D_Text* g_entries;
	int cursor;
	float offset;
	touchPosition currentPos;
} N(DataStruct);

// yutzo, diamonds, eddie

char* N(send_msg);
u32 N(send_batch_id);

SceneResult N(report)(Scene* sc, int i) {
	static const int msgmaxlen = 200;
	ReportListEntry* entry = &_data->list->entries[i];
	N(send_msg) = malloc(msgmaxlen + 1);
	SwkbdResult button;
	{
		char hint_text[50];
		char* mii_name[15];
		memset(mii_name, 0, sizeof(mii_name));
		utf16_to_utf8((u8*)mii_name, entry->mii.mii_name, 15);
		snprintf(hint_text, 50, _s(str_report_user_hint), mii_name);
		SwkbdState swkbd;
		memset(N(send_msg), 0, msgmaxlen + 1);
		swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, msgmaxlen);
		swkbdSetHintText(&swkbd, hint_text);
		swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, _s(str_cancel), false);
		swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, _s(str_submit), true);
		swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN | SWKBD_MULTILINE);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
		button = swkbdInputText(&swkbd, N(send_msg), msgmaxlen + 1);
	}
	if (button == SWKBD_D1_CLICK1) {
		// successfully submitted the input
		N(send_batch_id) = entry->batch_id;
		printf("Got report: \"%s\", sending...\n", N(send_msg));
		Scene* scene = getLoadingScene(0, lambda(void, (void) {
			CecMessageHeader msg;
			Result res = reportGetSomeMsgHeader(&msg, N(send_batch_id));
			if (R_FAILED(res)) {
				printf("ERROR: %lx\n", res);
				goto exit;
			}
			SHA256_HASH hash;
			Sha256Calculate(&msg, 0x28, &hash);
			ReportSendPayload* data = malloc(sizeof(ReportSendPayload));
			if (!data) goto exit;
			
			data->magic = 0x5053524e;
			data->version = 1;
			memcpy(data->message_id, msg.message_id, sizeof(CecMessageId));
			memcpy(&data->hash, &hash, sizeof(SHA256_HASH));
			memcpy(data->msg, N(send_msg), sizeof(data->msg));

			char url[50];
			snprintf(url, 50, "%s/report/new", BASE_URL);
			res = httpRequest("POST", url, sizeof(ReportSendPayload), (u8*)data, 0, 0);
			free(data);
			if (R_FAILED(res)) {
				printf("Error sending report: %ld\n", res);
				goto exit;
			}

			printf("report sent\n");
		exit:
			free(N(send_msg));
		}));
		scene->pop_scene = sc->pop_scene;
		sc->next_scene = scene;
		return scene_switch;
	}
	free(N(send_msg));
	return scene_pop;
}


void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;

	_data->list = loadReportList();

	if (!_data->list) {
		free(_data);
		sc->d = NULL;
		return;
	}

	_data->g_entries = malloc(sizeof(C2D_Text) * _data->list->header.cur_size);
	if (!_data->g_entries) {
		free(_data->list);
		free(_data);
		sc->d = NULL;
		return;
	}
	_data->cursor = -1;
	_data->offset = 0;
	_data->g_staticBuf = C2D_TextBufNew(40 * _data->list->header.cur_size);

	for (int i = 0; i < _data->list->header.cur_size; i++) {
		ReportListEntry* entry = &_data->list->entries[i];
		char mii_name[11] = {0};
		utf16_to_utf8((u8*)mii_name, entry->mii.mii_name, 11);
		C2D_TextFontParse(&_data->g_entries[i], getFontIndex(entry->mii.mii_options.char_set), _data->g_staticBuf, mii_name);
	}
	
	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_right_close;
	sc->setting.has_gradient = true;

	sc->app_state = app_idle;
}

void N(render_top)(Scene* sc) {
	if (!_data) {
		return;
	}

	// drawUser(&_data->list->entries[_data->cursor]);
	// u32 clr = C2D_Color32(0, 0, 0, 0xff);
	// for (int i = 0; i < _data->list->header.cur_size; i++) {
	// 	int ix = _data->list->header.cur_size - i - 1;
	// 	C2D_DrawText(&_data->g_entries[ix], C2D_AlignLeft | C2D_WithColor, 30, 35 + i*14 - _data->offset, 0, 0.5, 0.5, clr);
	// }
	// int x = 22;
	// int y = 35 + _data->cursor*14 + 3 - _data->offset;
	// C2D_DrawTriangle(x, y, clr, x, y +10, clr, x + 8, y + 5, clr, 1);
}

void N(render_bottom)(Scene* sc) {
	if (!_data) {
		return;
	}
	
	renderOptionButtons(_data->g_entries, _data->list->header.cur_size, _data->cursor, _data->offset, -1);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data->g_entries);
		free(_data->list);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	InputState state = sc->input_state;
	if (!_data) return scene_pop;

	// Update cursor
	_data->cursor += (state.k_down_repeat & KEY_DOWN && 1) - (state.k_down_repeat & KEY_UP && 1);
	_data->cursor += (state.k_down_repeat & KEY_RIGHT && 1)*10 - (state.k_down_repeat & KEY_LEFT && 1)*10;
	int list_max = (_data->list->header.cur_size - 1);
	if (state.k_down & (KEY_DOWN | KEY_UP)) {
		if (_data->cursor < 0) _data->cursor = list_max;
		if (_data->cursor > list_max) _data->cursor = 0;
	} else if (state.k_down_repeat & (KEY_DOWN | KEY_UP | KEY_RIGHT | KEY_LEFT)) {
		if (_data->cursor < 0) _data->cursor = 0;
		if (_data->cursor > list_max) _data->cursor = list_max;
	}

	// Update offset
	// while(_data->cursor*14 - _data->offset < 2) _data->offset--;
	// while(_data->cursor*14 - _data->offset > 180) _data->offset++;
	if (_data->cursor >= 0) {
		if (_data->cursor > _data->offset + 3) _data->offset = _data->cursor - 3;
		if (_data->cursor < _data->offset) _data->offset = _data->cursor;
	}

	if (state.k_up & KEY_TOUCH) {
		// Back button
		if (isRightButtonTouched(&state.pos_prev)) {
			return scene_pop;
		}
	}

	if (state.k_down & KEY_B) {
		if (_data->cursor < 0){
			return scene_pop;
		} else {
			_data->cursor = -1;
		}
	}
	
	if (state.k_down & KEY_A) {
		if (_data->cursor < 0) {
			_data->cursor = 0;
			return scene_continue;
		}

		int selected_i = _data->cursor;
		return N(report)(sc, selected_i);
	}
	if (state.k_down & KEY_START) {
		sc->app_state = app_exiting;
		return scene_continue;
	}
	return scene_continue;
}

Scene* getReportListScene(void) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}