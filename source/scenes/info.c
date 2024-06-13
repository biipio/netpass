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

#include "info.h"
#include "../render.h"
#include <stdlib.h>
#define N(x) scenes_scene_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

#define MARGIN 20
#define WIDTH (SCREEN_TOP_WIDTH - 2*MARGIN)
#define HEIGHT (SCREEN_TOP_HEIGHT - 2*MARGIN)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_info;
} N(DataStruct);

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	TextLangParse(&_data->g_info, _data->g_staticBuf, (void*)sc->data);
	
	Setting popSetting = sc->pop_scene->setting;
	sc->setting.bg_top = popSetting.bg_top;
	sc->setting.bg_bottom = popSetting.bg_bottom;
	sc->setting.btn_left = popSetting.btn_left;
	sc->setting.btn_right = popSetting.btn_right;
	sc->setting.has_gradient = popSetting.has_gradient;

	sc->app_state = sc->pop_scene->app_state;
}

void N(render_top)(Scene* sc) {
	C2D_DrawRectSolid(MARGIN, MARGIN, 0, WIDTH, HEIGHT, C2D_Color32(0xCC, 0xCC, 0xCC, 0xFF));
	C2D_DrawText(&_data->g_info, C2D_WithColor, MARGIN + 5, MARGIN + 5, 0, 0.5, 0.5, clr_black);
}

void N(render_bottom)(Scene* sc) {
	// C2D_DrawRectSolid(MARGIN, MARGIN, 1, WIDTH, HEIGHT, clr_overlay);
	// drawC2DText(&_data->g_info, clr_netpass_green, MARGIN + 5, MARGIN + 5, 1, 0.5, 0.5);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	if (sc->input_state.k_down & (KEY_A | KEY_B)) return scene_pop;
	return scene_continue;
}

Scene* getInfoScene(LanguageString s) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = (u32)s;
	scene->is_popup = true;
	scene->need_free = true;
	return scene;
}