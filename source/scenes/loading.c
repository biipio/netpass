#include "loading.h"
#include <stdlib.h>
#define N(x) scenes_loading_namespace_##x

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_loading;
	C2D_Text g_dots;
	C2D_SpriteSheet spr;
	float text_x;
	float text_y;
	float text_width;
	Thread thread;
	bool thread_done;
} N(DataStruct);

N(DataStruct)* N(data) = 0;

void N(threadFn)(Scene* sc) {
	((void(*)(void))(sc->data))();
	N(data)->thread_done = true;
}

void N(init)(Scene* sc) {
	N(data) = malloc(sizeof(N(DataStruct)));
	if (!N(data)) return;
	N(data)->g_staticBuf = C2D_TextBufNew(100);
	C2D_TextParse(&N(data)->g_loading, N(data)->g_staticBuf, "Loading    ");
	C2D_TextParse(&N(data)->g_dots, N(data)->g_staticBuf, "...");
	float height;
	C2D_TextGetDimensions(&N(data)->g_loading, 1, 1, &N(data)->text_width, &height);
	N(data)->text_x = (SCREEN_TOP_WIDTH - N(data)->text_width) / 2;
	N(data)->text_y = (SCREEN_TOP_HEIGHT - height) / 2;

	N(data)->spr = C2D_SpriteSheetLoad("romfs:/gfx/loading.t3x");

	N(data)->thread_done = false;
	N(data)->thread = threadCreate((void(*)(void*))N(threadFn), sc, 8*1024, main_thread_prio()-1, -2, false);
}

void N(render)(Scene* sc) {
	if (!N(data)) return;
	C2D_Image img = C2D_SpriteSheetGetImage(N(data)->spr, 0);
	C2D_DrawImageAt(img, 0, 0, 0, NULL, 1, 1);
	C2D_DrawText(&N(data)->g_loading, C2D_AlignLeft, N(data)->text_x, N(data)->text_y, 0, 1, 1);
	C2D_DrawText(&N(data)->g_dots, C2D_AlignLeft, N(data)->text_x + N(data)->text_width - 35 + 10*(time(NULL)%2), N(data)->text_y, 0, 1, 1);
}


void N(exit)(Scene* sc) {
	if (N(data)) {
		C2D_TextBufDelete(N(data)->g_staticBuf);
		C2D_SpriteSheetFree(N(data)->spr);
		threadJoin(N(data)->thread, U64_MAX);
		threadFree(N(data)->thread);
		free(N(data));
	}
}

SceneResult N(process)(Scene* sc) {
	if (N(data) && N(data)->thread_done) return scene_switch;
	return scene_continue;
}

Scene* getLoadingScene(Scene* next_scene, void(*func)(void)) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->next_scene = next_scene;
	scene->data = (u32)func;
	scene->need_free = true;
	return scene;
}