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
#include "../curl-handler.h"
#include "../render.h"
#include "../utils.h"
#include <stdlib.h>
#define N(x) scenes_settings_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

#define NUM_ENTRIES 7

typedef enum {MENU_DEFAULT, MENU_LANGUAGE, MENU_TIME_FORMAT} SettingsMenu;

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_title;
	C2D_Text g_entries[NUM_ENTRIES];
	C2D_Text g_languages[NUM_LANGUAGES + 1];
	C2D_Text g_timeFormats[2];
	C2D_Text g_versionText;
	int cursor;
	float offset;
	int selected_language;
	bool time_format_modified;
	SettingsMenu current_menu;
} N(DataStruct);

void N(load_language_text)(Scene* sc) {
	TextLangParse(&_data->g_languages[0], _data->g_staticBuf, str_system_language);
	for (int i = 0; i < NUM_LANGUAGES; i++) {
		TextLangSpecificParse(&_data->g_languages[i+1], _data->g_staticBuf, str_language, all_languages[i]);
	}
	_data->selected_language = -1;
	if (config.language != -1) {
		for (int i = 0; i < NUM_LANGUAGES; i++) {
			if (all_languages[i] == config.language) {
				_data->selected_language = i;
			}
		}
	}
}

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(500 + 15*NUM_LANGUAGES);
	_data->cursor = -1;
	_data->offset = 0;
	_data->selected_language = -2; // means uninitialized
	_data->current_menu = MENU_DEFAULT;
	TextLangParse(&_data->g_title, _data->g_staticBuf, str_settings);
	TextLangParse(&_data->g_entries[0], _data->g_staticBuf, str_toggle_titles);
	TextLangParse(&_data->g_entries[1], _data->g_staticBuf, str_report_user);
	TextLangParse(&_data->g_entries[2], _data->g_staticBuf, str_language_pick);
	TextLangParse(&_data->g_entries[3], _data->g_staticBuf, str_change_time_format);
	TextLangParse(&_data->g_entries[4], _data->g_staticBuf, str_download_data);
	TextLangParse(&_data->g_entries[5], _data->g_staticBuf, str_delete_data);
	TextLangParse(&_data->g_entries[6], _data->g_staticBuf, str_credits);

	TextLangParse(&_data->g_timeFormats[0], _data->g_staticBuf, str_24_hour);
	TextLangParse(&_data->g_timeFormats[1], _data->g_staticBuf, str_12_hour);
	
	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_right_close;

	sc->app_state = app_idle;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;
	
	C2D_DrawText(&_data->g_title, C2D_WithColor, 12, SCREEN_TOP_HEIGHT - 40, 0, 1.2, 1.2, clr_white);
	
	// C2D_DrawText(&_data->g_languages[_data->selected_language + 1], C2D_AlignLeft, 35 + _data->lang_width, 35 + 25, 0, 1, 1);

	// Trans flag
	{
		float x = SCREEN_TOP_WIDTH - 90;
		float y = SCREEN_TOP_HEIGHT - 60;
		u32 blue = C2D_Color32(0x2B, 0xCF, 0xFF, 0xFF);
		u32 pink = C2D_Color32(0xF5, 0xAB, 0xB9, 0xFF);
		u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
		C2D_DrawRectSolid(x, y, 0, 90, 12, blue);
		C2D_DrawRectSolid(x, y + 12, 0, 90, 12, pink);
		C2D_DrawRectSolid(x, y + 24, 0, 90, 12, white);
		C2D_DrawRectSolid(x, y + 36, 0, 90, 12, pink);
		C2D_DrawRectSolid(x, y + 48, 0, 90, 12, blue);
	}

	// Rainbow flag
	{
		float x = SCREEN_TOP_WIDTH - 180;
		float y = SCREEN_TOP_HEIGHT - 60;
		C2D_DrawRectSolid(x, y, 0, 90, 10, C2D_Color32(0xE5, 0x00, 0x00, 0xFF));
		C2D_DrawRectSolid(x, y + 10, 0, 90, 10, C2D_Color32(0xFF, 0x8D, 0x00, 0xFF));
		C2D_DrawRectSolid(x, y + 20, 0, 90, 10, C2D_Color32(0xFF, 0xEE, 0x00, 0xFF));
		C2D_DrawRectSolid(x, y + 30, 0, 90, 10, C2D_Color32(0x02, 0x81, 0x21, 0xFF));
		C2D_DrawRectSolid(x, y + 40, 0, 90, 10, C2D_Color32(0x00, 0x4C, 0xFF, 0xFF));
		C2D_DrawRectSolid(x, y + 50, 0, 90, 10, C2D_Color32(0x77, 0x00, 0x88, 0xFF));
	}
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;
	
	switch (_data->current_menu) {
	case MENU_DEFAULT:
		renderOptionButtons(_data->g_entries, NUM_ENTRIES, _data->cursor, _data->offset, -1);
		// TODO: render version and mini flags if they should be visible
		break;
	case MENU_LANGUAGE:
		if (_data->selected_language < -1) N(load_language_text)(sc);
		renderOptionButtons(_data->g_languages, NUM_LANGUAGES + 1, _data->cursor, _data->offset, -1);
		break;
	case MENU_TIME_FORMAT:
		renderOptionButtons(_data->g_timeFormats, 2, _data->cursor, _data->offset, -1);
		break;
	}
}

void N(exit)(Scene* sc) {
	if (_data) {
		if (_data->time_format_modified) {
			configWrite();
			_data->time_format_modified = false;
		}

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
	int list_max = NUM_ENTRIES;
	if (_data->current_menu == MENU_LANGUAGE) list_max = NUM_LANGUAGES;
	if (_data->current_menu == MENU_TIME_FORMAT) list_max = 2;
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
			switch (_data->current_menu) {
			case MENU_DEFAULT:
				return scene_pop;
			case MENU_LANGUAGE:
				_data->cursor = -1;
				_data->offset = 0;
				_data->current_menu = MENU_DEFAULT;
				break;
			case MENU_TIME_FORMAT:
				if (_data->time_format_modified) {
					configWrite();
					_data->time_format_modified = false;
				}
				_data->cursor = -1;
				_data->offset = 0;
				_data->current_menu = MENU_DEFAULT;
				break;
			}
		}
	}
	
	if (state.k_down & KEY_B) {
		if (_data->cursor < 0) {
			switch (_data->current_menu) {
			case MENU_DEFAULT:
				return scene_pop;
			case MENU_LANGUAGE:
				_data->cursor = -1;
				_data->offset = 0;
				_data->current_menu = MENU_DEFAULT;
				break;
			case MENU_TIME_FORMAT:
				if (_data->time_format_modified) {
					configWrite();
					_data->time_format_modified = false;
				}
				_data->cursor = -1;
				_data->offset = 0;
				_data->current_menu = MENU_DEFAULT;
				break;
			}
		} else {
			_data->cursor = -1;
		}
	}

		// if (_data->cursor == 1) {
		// 	int old_lang = _data->selected_language;
		// 	_data->selected_language += (kDown & KEY_RIGHT && 1) - (kDown & KEY_LEFT && 1);
		// 	if (_data->selected_language < -1) _data->selected_language = -1;
		// 	if (_data->selected_language > NUM_LANGUAGES-1) _data->selected_language = NUM_LANGUAGES-1;
		// 	if (old_lang != _data->selected_language) {
		// 		config.language = _data->selected_language == -1 ? -1 : all_languages[_data->selected_language];
		// 		configWrite();
		// 	}
		// }
		if (state.k_down & KEY_A) {
			if (_data->cursor < 0) {
				_data->cursor = 0;
				return scene_continue;
			}
	if (state.k_down & KEY_A) {
		if (_data->cursor < 0) {
			_data->cursor = 0;
			return scene_continue;
		}

		switch (_data->current_menu) {
		case MENU_DEFAULT:
		{
			if (_data->cursor == 0) {
				sc->next_scene = getToggleTitlesScene();
				return scene_push;
			}
			if (_data->cursor == 1) {
				sc->next_scene = getReportListScene();
				return scene_push;
			}
			if (_data->cursor == 2) {
				_data->cursor = _data->selected_language + 1;
				_data->offset = 0;
				_data->current_menu = MENU_LANGUAGE;
			}
			if (_data->cursor == 3) {
				_data->cursor = config.time_format;
				_data->offset = 0;
				_data->current_menu = MENU_TIME_FORMAT;
			}
			if (_data->cursor == 4) {
				sc->next_scene = getLoadingScene(0, lambda(void, (void) {
					char url[50];
					snprintf(url, 50, "%s/data", BASE_URL);
					Result res = httpRequest("GET", url, 0, 0, (void*)1, "/netpass_data.txt");
					if (R_FAILED(res)) {
						printf("ERROR downloading all data: %ld\n", res);
						return;
					}
					printf("Successfully downloaded all data!\n");
					printf("File stored at sdmc:/netpass_data.txt\n");
				}));
				return scene_push;
			}
			if (_data->cursor == 5) {
				sc->next_scene = getLoadingScene(0, lambda(void, (void) {
					char url[50];
					snprintf(url, 50, "%s/data", BASE_URL);
					Result res = httpRequest("DELETE", url, 0, 0, 0, 0);
					if (R_FAILED(res)) {
						printf("ERROR deleting all data: %ld\n", res);
						return;
					}
					printf("Successfully sent request to delete all data! This can take up to 15 days.\n");
				}));
				return scene_push;
			}
			// TODO: show credits when cursor == 6
			break;
		}
		
		case MENU_LANGUAGE:
		{
			break;
		}

		case MENU_TIME_FORMAT:
		{
			config.time_format = _data->cursor;
			_data->time_format_modified = true;
			break;
		}
		}
	}

	return scene_continue;
}

Scene* getSettingsScene(void) {
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
