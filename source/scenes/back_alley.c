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

#include "back_alley.h"
#include "../utils.h"
#include "../api.h"
#include "../render.h"
#include <stdlib.h>
#include <time.h>
#define N(x) scenes_back_alley_namespace_##x
#define _data ((N(DataStruct) *)sc->d)
#define MAX_PRICE 10

typedef struct
{
	C2D_TextBuf g_staticBuf;
	C2D_Text g_header;
	C2D_Text g_subtext;
	C2D_Text g_paytext;
	C2D_Text g_game_titles[24];
	C2D_Text g_not_implemented;
	u32 title_ids[24];
	C2D_Text g_back;
	PlayCoins *play_coins;
	int cursor;
	float offset;
	int number_games;
	bool show_games;
} N(DataStruct);

PlayCoins *N(play_coins);
u32 N(buy_title_id);

SceneResult N(buy_pass)(Scene *sc, int i)
{
	N(buy_title_id) = _data->title_ids[i];
	N(play_coins) = malloc(sizeof(PlayCoins));
	if (!N(play_coins))
	{
		return scene_continue;
	}
	memcpy(N(play_coins), _data->play_coins, sizeof(PlayCoins));

	Scene *scene = getLoadingScene(0, lambda(void, (void) {
		char url[80];
		snprintf(url, 80, "%s/pass/title_id/%lx", BASE_URL, N(buy_title_id));
		Result res = httpRequest("PUT", url, 0, 0, 0, 0);
		if (R_FAILED(res)) {
			if (res == -404) {
				printf("ERROR: No fitting pass found!");
				free(N(play_coins));
				return;
			}
			goto error;
		}
		N(play_coins)->total_coins -= config.price;
		config.price += 2;
		configWrite();
		if (config.price > 2) {
			Handle handle = 0;
			res = FSUSER_OpenFile(&handle, sharedextdata_b, fsMakePath(PATH_ASCII, "/gamecoin.dat"), FS_OPEN_WRITE, 0);
			if (R_FAILED(res)) goto error;
			u32 tmpval=0;
			res = FSFILE_Write(handle, &tmpval, 0, N(play_coins), sizeof(PlayCoins), FS_WRITE_FLUSH);
			FSFILE_Close(handle);
			if (R_FAILED(res)) goto error;
			free(N(play_coins));
		}
		triggerDownloadInboxes();
		return;
	error:
		printf("ERROR: failed processing pass: %lx\n", res);
		free(N(play_coins)); }));
	scene->pop_scene = sc->pop_scene;
	sc->next_scene = scene;
	return scene_switch;
}

void N(load_paytext)(C2D_Text *staticText, C2D_TextBuf staticBuf, int cost_amount)
{
	const char *s = _s(str_back_alley_pay);
	C2D_Font font = _font(str_back_alley_pay);
	char text[50];
	snprintf(text, 50, s, cost_amount);
	C2D_TextFontParse(staticText, font, staticBuf, text);
}

bool N(init_playcoins)(Scene *sc)
{
	_data->play_coins = malloc(sizeof(PlayCoins));
	if (!_data->play_coins)
	{
		free(_data);
		sc->d = 0;
		return false;
	}
	FILE *f = fopen("sharedextdata_b:/gamecoin.dat", "rb");
	if (!f)
	{
		free(_data->play_coins);
		free(_data);
		sc->d = 0;
		return false;
	}
	if (fread(_data->play_coins, sizeof(PlayCoins), 1, f) != 1)
	{
		fclose(f);
		free(_data->play_coins);
		free(_data);
		sc->d = 0;
		return false;
	}
	fclose(f);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (tm.tm_year + 1900 != config.year || tm.tm_mon + 1 != config.month || tm.tm_mday != config.day)
	{
		// our config is from the past day, time to set things up for the new day!
		config.price = 0;
		config.year = tm.tm_year + 1900;
		config.month = tm.tm_mon + 1;
		config.day = tm.tm_mday;
		configWrite();
	}
	return true;
}

bool N(init_gamelist)(Scene *sc)
{
	Result res = 0;
	CecMboxListHeader mbox_list;
	res = cecdOpenAndRead(0, CEC_PATH_MBOX_LIST, sizeof(CecMboxListHeader), (u8 *)&mbox_list);
	if (R_FAILED(res))
		return false;
	clearIgnoredTitles(&mbox_list);
	_data->number_games = mbox_list.num_boxes;
	u16 title_name_utf16[50];
	char title_name[50];
	for (int i = 0; i < _data->number_games; i++)
	{
		u32 title_id = strtol((const char *)mbox_list.box_names[i], NULL, 16);
		memset(title_name_utf16, 0, sizeof(title_name_utf16));
		memset(title_name, 0, sizeof(title_name));
		res = cecdOpenAndRead(title_id, CECMESSAGE_BOX_TITLE, sizeof(title_name_utf16), (u8 *)title_name_utf16);
		if (R_FAILED(res))
			return false;
		utf16_to_utf8((u8 *)title_name, title_name_utf16, sizeof(title_name));
		char *ptr = title_name;
		while (*ptr)
		{
			if (*ptr == '\n')
				*ptr = ' ';
			ptr++;
		}
		_data->title_ids[i] = title_id;
		C2D_TextParse(&_data->g_game_titles[i], _data->g_staticBuf, title_name);
	}
	return true;
}

void N(init)(Scene *sc)
{
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data)
		return;

	if (!N(init_playcoins)(sc))
		return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	if (!N(init_gamelist)(sc))
	{
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data->play_coins);
		free(_data);
		sc->d = 0;
		return;
	}

	_data->cursor = -1;
	_data->offset = 0;
	_data->show_games = false;
	TextLangParse(&_data->g_header, _data->g_staticBuf, str_back_alley);
	TextLangParse(&_data->g_subtext, _data->g_staticBuf, str_back_alley_message);
	N(load_paytext)
	(&_data->g_paytext, _data->g_staticBuf, config.price > MAX_PRICE ? 0 : config.price);
	TextLangParse(&_data->g_back, _data->g_staticBuf, str_back);
	C2D_TextParse(&_data->g_not_implemented, _data->g_staticBuf, "This is not implemented yet.");

	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_right_close;

	sc->app_state = app_idle;
}

void N(render_top)(Scene *sc)
{
	if (!_data)
		return;
	u32 clr = C2D_Color32(0, 0, 0, 0xff);
	C2D_DrawText(&_data->g_header, C2D_AlignLeft | C2D_WithColor, 10, 10, 0, 1, 1, clr);
	C2D_DrawText(&_data->g_subtext, C2D_AlignLeft | C2D_WithColor, 11, 35, 0, 0.5, 0.5, clr);
	if (_data->show_games)
	{
		// int i = 0;
		// for (; i < _data->number_games; i++) {
		// 	C2D_DrawText(&_data->g_game_titles[i], C2D_AlignLeft | C2D_WithColor, 30, 55 + (i*14), 0, 0.5, 0.5, clr);
		// }
		// C2D_DrawText(&_data->g_back, C2D_AlignLeft | C2D_WithColor, 30, 55 + (i*14), 0, 0.5, 0.5, clr);

		// int x = 22;
		// int y = _data->cursor*14 + 55 + 3;
		// C2D_DrawTriangle(x, y, clr, x, y +10, clr, x + 8, y + 5, clr, 1);
	}
	else
	{
		// bool grayed_out = config.price > MAX_PRICE || config.price > _data->play_coins->total_coins;
		// C2D_DrawText(&_data->g_paytext, C2D_AlignLeft | C2D_WithColor, 30, 55, 0, 1, 1, grayed_out ? C2D_Color32(0, 0, 0, 0x80) : clr);
		// C2D_DrawText(&_data->g_back, C2D_AlignLeft | C2D_WithColor, 30, 80, 0, 1, 1, clr);

		// int x = 10;
		// int y = _data->cursor*25 + 55 + 5;
		// C2D_DrawTriangle(x, y, clr, x, y + 18, clr, x + 15, y + 9, clr, 1);
	}
}

void N(render_bottom)(Scene *sc)
{
	if (!_data)
		return;

	if (_data->show_games)
	{
		renderOptionButtons(_data->g_game_titles, _data->number_games, _data->cursor, _data->offset, -1);
	}
	else
	{
		renderOptionButtons(&_data->g_paytext, 1, _data->cursor, _data->offset, -1);
	}
}

void N(exit)(Scene *sc)
{
	if (_data)
	{
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene *sc)
{
	InputState state = sc->input_state;
	if (_data)
	{
		// Update cursor
		_data->cursor += (state.k_down_repeat & KEY_DOWN && 1) - (state.k_down_repeat & KEY_UP && 1);
		_data->cursor += (state.k_down_repeat & KEY_RIGHT && 1) * 10 - (state.k_down_repeat & KEY_LEFT && 1) * 10;
		int list_max = _data->show_games ? (_data->number_games - 1) : 0;
		if (state.k_down & (KEY_DOWN | KEY_UP))
		{
			if (_data->cursor < 0)
				_data->cursor = list_max;
			if (_data->cursor > list_max)
				_data->cursor = 0;
		}
		else if (state.k_down_repeat & (KEY_DOWN | KEY_UP | KEY_RIGHT | KEY_LEFT))
		{
			if (_data->cursor < 0)
				_data->cursor = 0;
			if (_data->cursor > list_max)
				_data->cursor = list_max;
		}

		// Update offset
		if (_data->cursor >= 0)
		{
			// TODO: treat as pixel, not list index
			if (_data->cursor > _data->offset + 3)
				_data->offset = _data->cursor - 3;
			if (_data->cursor < _data->offset)
				_data->offset = _data->cursor;
		}

		if (_data->show_games)
		{
			if (state.k_up & KEY_TOUCH)
			{
				// Close button
				if (isRightButtonTouched(&state.pos_prev))
				{
					// Go back
					_data->cursor = -1;
					_data->offset = 0;
					_data->show_games = false;
				}
			}

			if (state.k_down & KEY_B)
			{
				if (_data->cursor < 0)
				{
					// Go back
					_data->cursor = -1;
					_data->offset = 0;
					_data->show_games = false;
				}
				else
				{
					_data->cursor = -1;
				}
			}

			if (state.k_down & KEY_A)
			{
				if (_data->cursor < 0)
				{
					_data->cursor = 0;
					return scene_continue;
				}

				// picked a game
				return scene_continue;
				// return N(buy_pass)(sc, _data->cursor);
			}
		}
		else
		{
			if (state.k_up & KEY_TOUCH)
			{
				// Close button
				if (isRightButtonTouched(&state.pos_prev))
				{
					return scene_pop;
				}
			}

			if (state.k_down & KEY_B)
			{
				if (_data->cursor < 0)
				{
					return scene_pop;
				}
				else
				{
					_data->cursor = -1;
				}
			}

			if (state.k_down & KEY_A)
			{
				if (_data->cursor < 0)
				{
					_data->cursor = 0;
					return scene_continue;
				}

				if (_data->cursor == 0 && config.price <= MAX_PRICE && config.price <= _data->play_coins->total_coins)
				{
					_data->cursor = -1;
					_data->offset = 0;
					_data->show_games = true;
				}
			}
		}
	}
	if (state.k_down & KEY_START)
	{
		sc->app_state = app_exiting;
		return scene_continue;
	}
	return scene_continue;
}

Scene *getBackAlleyScene()
{
	Scene *scene = malloc(sizeof(Scene));
	if (!scene)
		return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}
