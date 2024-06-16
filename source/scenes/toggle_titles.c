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
	C2D_Text g_back;
	int cursor;
	float offset;
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
	
	_data->cursor = 0;
	TextLangParse(&_data->g_header, _data->g_staticBuf, str_toggle_titles);
	TextLangParse(&_data->g_subtext, _data->g_staticBuf, str_toggle_titles_message);
	TextLangParse(&_data->g_back, _data->g_staticBuf, str_back);
	TextLangParse(&_data->g_on_off[0], _data->g_staticBuf, str_toggle_titles_off);
	TextLangParse(&_data->g_on_off[1], _data->g_staticBuf, str_toggle_titles_on);
	
	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_right_close;

	sc->app_state = app_idle;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;
	u32 clr = C2D_Color32(0, 0, 0, 0xff);
	u32 onClr = C2D_Color32(10, 200, 10, 0xff);
	u32 offClr = C2D_Color32(200, 10, 10, 0xff);
	C2D_DrawText(&_data->g_header, C2D_AlignLeft | C2D_WithColor, 10, 10, 0, 1, 1, clr);
	C2D_DrawText(&_data->g_subtext, C2D_AlignLeft | C2D_WithColor, 11, 40, 0, 0.5, 0.5, clr);
	int i = 0;
	for (; i < _data->number_games; i++) {
		bool isIgnored = isTitleIgnored(_data->title_ids[i]);
		u32 correctClr = isIgnored ? offClr : onClr;
		C2D_DrawText(&_data->g_on_off[!isIgnored], C2D_AlignLeft | C2D_WithColor, 30, 60 + (i * 14), 0, 0.5, 0.5, correctClr);
		C2D_DrawText(&_data->g_game_titles[i], C2D_AlignLeft | C2D_WithColor, 70, 60 + (i * 14), 0, 0.5, 0.5, correctClr);
	}
	C2D_DrawText(&_data->g_back, C2D_AlignLeft | C2D_WithColor, 30, 60 + (i*14), 0, 0.5, 0.5, clr);
	
	int x = 22;
	int y = _data->cursor*14 + 60 + 3;
	C2D_DrawTriangle(x, y, clr, x, y + 10, clr, x + 8, y + 5, clr, 1);
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
}

void N(exit)(Scene* sc) {
	configWrite();
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	InputState state = sc->input_state;
	if (!_data) return scene_continue;

	// Update cursor
	_data->cursor += (state.k_down_repeat & KEY_DOWN && 1) - (state.k_down_repeat & KEY_UP && 1);
	_data->cursor += (state.k_down_repeat & KEY_RIGHT && 1)*4 - (state.k_down_repeat & KEY_LEFT && 1)*4;
	int list_max = _data->number_games;
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

	if (state.k_up & KEY_TOUCH) {
		// Back button
		if (isRightButtonTouched(&state.pos_prev)) {
			return scene_pop;
		}
	}

	if (state.k_down & KEY_B) {
		return scene_pop;
	}

	if (state.k_down & KEY_A) {
		// "Back" is selected, exit this scene
		if (_data->cursor == _data->number_games) {
			return scene_pop;
		}

		// Add title to ignore list / Remove title from ignore list
		u32 title_id = _data->title_ids[_data->cursor];
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
