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

#include "loading.h"
#include "../render.h"
#include <stdlib.h>
#define N(x) scenes_loading_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	// C2D_TextBuf g_staticBuf;
	Thread thread;
	bool thread_done;
} N(DataStruct);

void N(threadFn)(Scene* sc) {
	((void(*)(void))(sc->data))();
	_data->thread_done = true;
	sc->app_state = app_idle;
}

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	// _data->g_staticBuf = C2D_TextBufNew(100);

	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_empty;

	_data->thread_done = false;
	_data->thread = threadCreate((void(*)(void*))N(threadFn), sc, 8*1024, main_thread_prio()-1, -2, false);

	sc->app_state = app_loading;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;

	renderLogo((SCREEN_TOP_WIDTH - 300)/2, (SCREEN_TOP_HEIGHT - 86)/2, 0);
	// C2D_DrawText(&_data->g_loading, C2D_AlignLeft, _data->text_x, _data->text_y, 0, 1, 1);
	// C2D_DrawText(&_data->g_dots, C2D_AlignLeft, _data->text_x + _data->text_width - 35 + 10*(time(NULL)%2), _data->text_y, 0, 1, 1);
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	// C2D_Image img = C2D_SpriteSheetGetImage(_data->spr, 0);
	// C2D_DrawImageAt(img, 0, 0, 0, NULL, 1, 1);
	// C2D_DrawText(&_data->g_loading, C2D_AlignLeft, _data->text_x, _data->text_y, 0, 1, 1);
	// C2D_DrawText(&_data->g_dots, C2D_AlignLeft, _data->text_x + _data->text_width - 35 + 10*(time(NULL)%2), _data->text_y, 0, 1, 1);
}

void N(exit)(Scene* sc) {
	if (_data) {
		// C2D_TextBufDelete(_data->g_staticBuf);
		threadJoin(_data->thread, U64_MAX);
		threadFree(_data->thread);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	if (_data && _data->thread_done) {
		if (sc->next_scene) return scene_switch;
		return scene_pop;
	}
	return scene_continue;
}

Scene* getLoadingScene(Scene* next_scene, void(*func)(void)) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->next_scene = next_scene;
	scene->data = (u32)func;
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}