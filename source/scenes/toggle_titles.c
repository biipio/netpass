/**
 * NetPass
 * Copyright (C) 2024 SunOfLife1
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

#include "toggle_titles.h"
#include "../config.h"
#include "../render.h"
#include "../utils.h"
#include <stdlib.h>

#define N(x) scenes_toggle_titles_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_header;
	C2D_Text g_subtext;
	C2D_Text g_on_off[2];
	C2D_Text g_game_titles[24];
	u32 title_ids[24];
	int number_games;
} N(DataStruct);

bool N(init_gamelist)(Scene* sc) {
	Result res = 0;
	CecMboxListHeader mbox_list;
	res = cecdOpenAndRead(0, CEC_PATH_MBOX_LIST, sizeof(CecMboxListHeader), (u8*)&mbox_list);
	if (R_FAILED(res)) return false;
	_data->number_games = mbox_list.num_boxes;
	u16 title_name_utf16[50];
	char title_name[50];
	for (int i = 0; i < _data->number_games; i++) {
		u32 title_id = strtol((const char*)mbox_list.box_names[i], NULL, 16);
		memset(title_name_utf16, 0, sizeof(title_name_utf16));
		memset(title_name, 0, sizeof(title_name));
		res = cecdOpenAndRead(title_id, CECMESSAGE_BOX_TITLE, sizeof(title_name_utf16), (u8*)title_name_utf16);
		if (R_FAILED(res)) return false;
		utf16_to_utf8((u8*)title_name, title_name_utf16, sizeof(title_name));
		char* ptr = title_name;
		while (*ptr) {
			if (*ptr == '\n') *ptr = ' ';
			ptr++;
		}
		_data->title_ids[i] = title_id;
		C2D_TextParse(&_data->g_game_titles[i], _data->g_staticBuf, title_name);
	}
	return true;
}

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	
	_data->g_staticBuf = C2D_TextBufNew(2000);
	if (!N(init_gamelist)(sc)) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
		sc->d = 0;
		return;
	}
	
	TextLangParse(&_data->g_header, _data->g_staticBuf, str_toggle_titles);
	TextLangParse(&_data->g_subtext, _data->g_staticBuf, str_toggle_titles_message);
	TextLangParse(&_data->g_on_off[0], _data->g_staticBuf, str_toggle_titles_off);
	TextLangParse(&_data->g_on_off[1], _data->g_staticBuf, str_toggle_titles_on);
	
	sc->setting.btn_count = _data->number_games;
	sc->setting.btn_cursor = -1;
	sc->setting.scroll_offset = 0;
	sc->setting.scroll_velocity = 0;
	sc->input_state.isTouched = false;

	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_left_help;
	sc->setting.btn_right = ui_btn_right_close;

	sc->setting.btn_colors = NULL;
	resetBtnColors(&sc->setting.btn_colors, sc->setting.btn_count, clr_netpass_green);
}

void N(render_top)(Scene* sc) {
	if (!_data) return;

	if (sc->setting.btn_cursor < 0) {
		// Render "Toggle Games" title
		renderTextWithOutline(
			&_data->g_header, C2D_WithColor,
			12, SCREEN_TOP_HEIGHT - 40, 0,
			1.2f, 1.2f, 1.75f,
			clr_white, clr_netpass_green
		);
		// TODO: render subtext
	} else {
		float x = CENTER_TOP_X(368);
		float y = CENTER_TOP_Y(162); // Image height is 183, top bar height is 21, 162 = 183 - 21

		// Render info box
		renderImage(spr_misc, ui_misc_info_box, x, y, 0);

		// Render game title
		C2D_DrawText(&_data->g_game_titles[sc->setting.btn_cursor], C2D_WithColor, x + 10, y + 10, 0, 0.75f, 0.75f, clr_white);
	}
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	
	for (size_t i = 0; i < sc->setting.btn_count; i++) {
		if (isTitleIgnored(_data->title_ids[i])) {
			sc->setting.btn_colors[i] = clr_off_red;
		} else {
			sc->setting.btn_colors[i] = clr_netpass_green;
		}
	}
}

void N(exit)(Scene* sc) {
	configWrite();
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
		free(sc->setting.btn_colors);
	}
}

SceneResult N(process)(Scene* sc) {
	app_state = app_idle;
	InputState state = sc->input_state;
	Setting* setting = &sc->setting;
	if (!_data) return scene_continue;

	// Update cursor and offset
	updateListCursor(&setting->btn_cursor, &state, _data->number_games - 1);
	updateListOffset(&setting->scroll_offset, &setting->scroll_velocity, &setting->btn_cursor, setting->btn_count, &state);

	if (state.k_up & KEY_TOUCH) {
		// Help button
		if (isLeftButtonTapped(&state.pos_start, &state.pos_prev)) {
			// TODO: implement this
		}

		// Back button
		if (isRightButtonTapped(&state.pos_start, &state.pos_prev)) {
			return scene_pop;
		}
	}

	if (state.k_down & KEY_B) {
		if (setting->btn_cursor < 0) {
			return scene_pop;
		} else {
			setting->btn_cursor = -1;
		}
	}

	if (state.k_down & KEY_A) {
		if (setting->btn_cursor < 0) {
			setting->btn_cursor = 0;
			return scene_continue;
		}

		// Add title to ignore list / Remove title from ignore list
		u32 title_id = _data->title_ids[setting->btn_cursor];
		if (isTitleIgnored(title_id)) {
			removeIgnoredTitle(title_id);
		} else {
			addIgnoredTitle(title_id);
		}
		return scene_continue;
	}

	return scene_continue;
}

Scene* getToggleTitlesScene(void) {
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
