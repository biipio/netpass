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
	C2D_Text g_locations[NUM_LOCATIONS];
} N(DataStruct);

static int location_random[NUM_LOCATIONS];

static inline int location_real_to_random(int real) {
	for (size_t i = 0; i < NUM_LOCATIONS; i++)
		if (location_random[i] == real) return i;
	
	return -1;
}

void N(randomize_locations)(Scene* sc) {
	psInit();
	
	for (size_t i = 0; i < NUM_LOCATIONS; i++) {
		location_random[i] = i;
	}

	u8 rand;
	for (size_t i = 0; i < NUM_LOCATIONS; i++) {
		PS_GenerateRandomBytes(&rand, 8);
		u8 target = rand % NUM_LOCATIONS;

		int temp = location_random[i];
		location_random[i] = location_random[target];
		location_random[target] = temp;

		C2D_Text tempText = _data->g_locations[i];
		_data->g_locations[i] = _data->g_locations[target];
		_data->g_locations[target] = tempText;
	}

	psExit();
}

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	TextLangParse(&_data->g_home, _data->g_staticBuf, str_home);
	TextLangParse(&_data->g_choose, _data->g_staticBuf, str_choose_location);
	TextLangParse(&_data->g_locations[0], _data->g_staticBuf, str_train_station);
	TextLangParse(&_data->g_locations[1], _data->g_staticBuf, str_plaza);
	TextLangParse(&_data->g_locations[2], _data->g_staticBuf, str_mall);
	TextLangParse(&_data->g_locations[3], _data->g_staticBuf, str_beach);
	TextLangParse(&_data->g_locations[4], _data->g_staticBuf, str_arcade);
	TextLangParse(&_data->g_locations[5], _data->g_staticBuf, str_catcafe);

	sc->setting.btn_count = NUM_LOCATIONS;
	sc->setting.btn_cursor = -1;
	sc->setting.scroll_offset = 0;
	sc->setting.scroll_velocity = 0;
	sc->input_state.isTouched = false;

	N(randomize_locations)(sc);

	sc->setting.bg_top = bg_top_home;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_left_help;
	sc->setting.btn_right = ui_btn_right_settings;
	sc->setting.use_previews = true;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;

	if (sc->setting.btn_cursor < 0) {
		renderTextWithOutline(
			&_data->g_home, 0,
			12, SCREEN_TOP_HEIGHT - 40, 0,
			1.2f, 1.2f, 1.75f,
			clr_white, clr_netpass_green
		);
	} else {
		renderTextWithOutline(
			&_data->g_choose, C2D_AlignCenter,
			SCREEN_TOP_WIDTH/2, SCREEN_TOP_HEIGHT - 40, 0,
			1.2f, 1.2f, 1.75f,
			clr_white, clr_netpass_green
		);
	}
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	
	renderOptionButtons(_data->g_locations, sc->setting.btn_count, sc->setting.btn_cursor, sc->setting.scroll_offset, location_real_to_random(config.last_location));
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

Result N(location_res);

SceneResult N(process)(Scene* sc) {
	app_state = app_idle;
	InputState state = sc->input_state;
	Setting* setting = &sc->setting;
	if (!_data) return scene_continue;


	// Update cursor and offset
	updateListCursor(&setting->btn_cursor, &state, setting->btn_count - 1);
	updateListOffset(&setting->scroll_offset, &setting->scroll_velocity, &setting->btn_cursor, setting->btn_count, &state);

	int real_cursor_location = (setting->btn_cursor < 0) ? -1 : location_random[setting->btn_cursor];
	
	// Change background depending on currently hovered location
	setting->bg_top = real_cursor_location + 2;
	setting->bg_bottom = bg_bottom_generic;

	if (state.k_up & KEY_TOUCH) {
		// Help button
		if (isLeftButtonTapped(&state.pos_start, &state.pos_prev)) {
			// TODO: implement this
		}

		// Settings button
		if (isRightButtonTapped(&state.pos_start, &state.pos_prev)) {
			sc->next_scene = getSettingsScene();
			return scene_push;
		}
	}
	
	if (state.k_down & KEY_B) {
		setting->btn_cursor = -1;
	}

	if (state.k_down & KEY_A) {
		if (setting->btn_cursor < 0) {
			setting->btn_cursor = 0;
			return scene_continue;
		}

		// load location scene
		if (real_cursor_location == config.last_location) {
			sc->next_scene = getInfoScene(str_no_location_twice);
			return scene_push;
		}
		location = real_cursor_location;

		sc->next_scene = getLoadingScene(getSwitchScene(lambda(Scene*, (void) {
		// 	if (R_FAILED(N(location_res))) return getHomeScene();
			return getLocationScene(location);
		})), lambda(void, (void) {
		// 	N(location_res) = setLocation(location);
		// 	triggerDownloadInboxes();
		}));
		return scene_switch;
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