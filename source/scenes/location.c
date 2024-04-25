#include "switch.h"
#include <stdlib.h>
#define N(x) scenes_location_namespace_##x

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_location;
	C2D_Text g_entries[1];
	int cursor;
} N(DataStruct);

N(DataStruct)* N(data) = 0;
LanguageString* N(locations)[3] = {
	&str_at_train_station,
	&str_at_plaza,
	&str_at_mall,
};

void N(init)(Scene* sc) {
	N(data) = malloc(sizeof(N(DataStruct)));
	if (!N(data)) return;
	N(data)->g_staticBuf = C2D_TextBufNew(2000);
	N(data)->cursor = 0;
	TextLangParse(&N(data)->g_location, N(data)->g_staticBuf, *N(locations)[sc->data]);
	TextLangParse(&N(data)->g_entries[0], N(data)->g_staticBuf, str_exit);
}

void N(render)(Scene* sc) {
	if (!N(data)) return;
	C2D_DrawText(&N(data)->g_location, C2D_AlignLeft, 10, 10, 0, 1, 1);
	for (int i = 0; i < 1; i++) {
		C2D_DrawText(&N(data)->g_entries[i], C2D_AlignLeft, 30, 10 + (i+1)*25, 0, 1, 1);
	}
	u32 clr = C2D_Color32(0, 0, 0, 0xff);
	int x = 10;
	int y = 10 + (N(data)->cursor + 1)*25 + 5;
	C2D_DrawTriangle(x, y, clr, x, y + 18, clr, x + 15, y + 9, clr, 1);
}

void N(exit)(Scene* sc) {
	if (N(data)) {
		C2D_TextBufDelete(N(data)->g_staticBuf);
		free(N(data));
	}
}

SceneResult N(process)(Scene* sc) {
	hidScanInput();
	u32 kDown = hidKeysDown();
	if (N(data)) {
		N(data)->cursor += ((kDown & KEY_DOWN || kDown & KEY_CPAD_DOWN) && 1) - ((kDown & KEY_UP || kDown & KEY_CPAD_UP) && 1);
		if (N(data)->cursor < 0) N(data)->cursor = 0;
		if (N(data)->cursor > 0) N(data)->cursor = 0;
		if (kDown & KEY_A) {
			return scene_stop;
		}
	}
	if (kDown & KEY_START) return scene_stop;
	return scene_continue;
}

Scene* getLocationScene(int location) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = location;
	scene->need_free = true;
	return scene;
}