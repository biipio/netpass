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

#include "home.h"
#include <stdlib.h>
#include "../api.h"
#include "../render.h"
#include "../utils.h"
#define N(x) scenes_home_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_home;
	C2D_Text g_choose;
	C2D_Text g_entries[NUM_LOCATIONS];
	int cursor;
	float offset;
} N(DataStruct);

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	_data->cursor = -1;
	_data->offset = 0;
	TextLangParse(&_data->g_home, _data->g_staticBuf, str_home);
	TextLangParse(&_data->g_choose, _data->g_staticBuf, str_choose_location);
	TextLangParse(&_data->g_entries[0], _data->g_staticBuf, str_train_station);
	TextLangParse(&_data->g_entries[1], _data->g_staticBuf, str_plaza);
	TextLangParse(&_data->g_entries[2], _data->g_staticBuf, str_mall);
	TextLangParse(&_data->g_entries[3], _data->g_staticBuf, str_beach);
	TextLangParse(&_data->g_entries[4], _data->g_staticBuf, str_arcade);
	TextLangParse(&_data->g_entries[5], _data->g_staticBuf, str_catcafe);

	sc->setting.bg_top = bg_top_home;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_left_home;
	sc->setting.btn_right = ui_btn_right_settings;
	sc->setting.has_gradient = true;
	sc->setting.use_previews = true;

	sc->app_state = app_idle;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;

	if (_data->cursor < 0) {
		C2D_DrawText(&_data->g_home, C2D_WithColor, 12, SCREEN_TOP_HEIGHT - 40, 0, 1.2f, 1.2f, clr_white);
	} else {
		u32 flags = C2D_AlignCenter | C2D_WithColor;
		C2D_DrawText(&_data->g_choose, flags, SCREEN_TOP_WIDTH/2, SCREEN_TOP_HEIGHT - 40, 0, 1.2f, 1.2f, clr_white);
	}
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	
	renderOptionButtons(_data->g_entries, NUM_LOCATIONS, _data->cursor, _data->offset, config.last_location);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

Result N(location_res);

SceneResult N(process)(Scene* sc) {
	InputState state = sc->input_state;
	if (_data) {
		// Update cursor
		_data->cursor += (state.k_down_repeat & KEY_DOWN && 1) - (state.k_down_repeat & KEY_UP && 1);
		_data->cursor += (state.k_down_repeat & KEY_RIGHT && 1)*10 - (state.k_down_repeat & KEY_LEFT && 1)*10;
		int list_max = (NUM_LOCATIONS - 1);
		if (state.k_down & (KEY_DOWN | KEY_UP)) {
			if (_data->cursor < 0) _data->cursor = list_max;
			if (_data->cursor > list_max) _data->cursor = 0;
		} else if (state.k_down_repeat & (KEY_DOWN | KEY_UP | KEY_RIGHT | KEY_LEFT)) {
			if (_data->cursor < 0) _data->cursor = 0;
			if (_data->cursor > list_max) _data->cursor = list_max;
		}

		// Update offset
		if (_data->cursor >= 0) {
			// TODO: treat as pixel, not list index
			if (_data->cursor > _data->offset + 3) _data->offset = _data->cursor - 3;
			if (_data->cursor < _data->offset) _data->offset = _data->cursor;
		}
		
		// Change background depending on currently hovered location
		sc->setting.bg_top = _data->cursor + 2;
		sc->setting.bg_bottom = bg_bottom_generic;

		if (state.k_up & KEY_TOUCH) {
			// Home button
			if (isLeftButtonTouched(&state.pos_prev)) {
				sc->app_state = app_exiting;
				return scene_continue;
			}

			// Settings button
			if (isRightButtonTouched(&state.pos_prev)) {
				sc->next_scene = getSettingsScene();
				return scene_push;
			}
		}
		
		if (state.k_down & KEY_B) {
			_data->cursor = -1;
		}

		if (state.k_down & KEY_A) {
			if (_data->cursor < 0) {
				_data->cursor = 0;
				return scene_continue;
			}

			// load location scene
			if (_data->cursor == config.last_location) {
				sc->next_scene = getInfoScene(str_no_location_twice);
				return scene_push;
			}
			location = _data->cursor;

			sc->next_scene = getLoadingScene(getSwitchScene(lambda(Scene*, (void) {
			// 	if (R_FAILED(N(location_res))) return getHomeScene();
				return getLocationScene(location);
			})), lambda(void, (void) {
			// 	N(location_res) = setLocation(location);
			// 	triggerDownloadInboxes();
			}));
			return scene_switch;
		}
	}
	if (state.k_down & KEY_START) {
		sc->app_state = app_exiting;
		return scene_continue;
	}
	if (state.k_down & KEY_SELECT) {
		sc->next_scene = getSettingsScene();
		return scene_push;
	}
	return scene_continue;
}

Scene* getHomeScene(void) {
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