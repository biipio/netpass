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
#include "../api.h"
#include "../render.h"
#include "../utils.h"
#include <stdlib.h>
#define N(x) scenes_location_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

#define NUM_ENTRIES 2

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_location;
	C2D_Text g_entries[NUM_ENTRIES];
	int cursor;
	touchPosition currentPos;
} N(DataStruct);

LanguageString* N(locations)[NUM_LOCATIONS] = {
	&str_train_station,
	&str_plaza,
	&str_mall,
	&str_beach,
	&str_arcade,
	&str_catcafe,
};

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	_data->cursor = -1;
	TextLangParse(&_data->g_location, _data->g_staticBuf, *N(locations)[sc->data]);
	TextLangParse(&_data->g_entries[0], _data->g_staticBuf, str_check_inboxes);
	TextLangParse(&_data->g_entries[1], _data->g_staticBuf, str_exit);

	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = sc->data + 2;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_left_back_alley;
	sc->setting.btn_right = ui_btn_right_settings;
	sc->setting.has_gradient = true;

	sc->app_state = app_idle;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;
	
	C2D_DrawText(&_data->g_location, C2D_WithColor, 12, SCREEN_TOP_HEIGHT - 40, 0, 1.2, 1.2, clr_white);
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	
	renderOptionButtons(_data->g_entries, NUM_ENTRIES, _data->cursor, 0, -1);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	InputState state = sc->input_state;
	if (_data) {
		// Update cursor
		_data->cursor += (state.k_down_repeat & KEY_DOWN && 1) - (state.k_down_repeat & KEY_UP && 1);
		_data->cursor += (state.k_down_repeat & KEY_RIGHT && 1)*10 - (state.k_down_repeat & KEY_LEFT && 1)*10;
		int list_max = (NUM_ENTRIES - 1);
		if (state.k_down & (KEY_DOWN | KEY_UP)) {
			if (_data->cursor < 0) _data->cursor = list_max;
			if (_data->cursor > list_max) _data->cursor = 0;
		} else if (state.k_down_repeat & (KEY_DOWN | KEY_UP | KEY_RIGHT | KEY_LEFT)) {
			if (_data->cursor < 0) _data->cursor = 0;
			if (_data->cursor > list_max) _data->cursor = list_max;
		}

		if (state.k_up & KEY_TOUCH) {
			// Back alley button
			if (isLeftButtonTouched(&state.pos_prev)) {
				sc->next_scene = getBackAlleyScene();
				return scene_push;
			}
			
			// Settings button
			if (isRightButtonTouched(&state.pos_prev)) {
				sc->next_scene = getSettingsScene();
				return scene_push;
			}
		}

		if (state.k_down & KEY_B) {
			if (_data->cursor < 0) {
				sc->next_scene = getLoadingScene(getSwitchScene(lambda(Scene*, (void) {
					return getHomeScene();
				})), lambda(void, (void) {}));
				return scene_switch;
			} else {
				_data->cursor = -1;
			}
			// _data->cursor = -1;
		}

		if (state.k_down & KEY_A) {
			if (_data->cursor < 0) {
				_data->cursor = 0;
				return scene_continue;
			}

			switch (_data->cursor) {
				case 0:
					sc->next_scene = getLoadingScene(0, lambda(void, (void) {
						// downloadInboxes();
					}));
					return scene_push;
				case 1:
					sc->next_scene = getLoadingScene(getSwitchScene(lambda(Scene*, (void) {
						return getHomeScene();
					})), lambda(void, (void) {}));
					return scene_switch;
					// sc->app_state = app_exiting;
					// return scene_continue;
			}
		}
	}
	
	return scene_continue;
}

Scene* getLocationScene(int location) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = location;
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}