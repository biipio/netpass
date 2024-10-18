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

#include "connection_error.h"
#include "../render.h"
#include <stdlib.h>
#include <curl/curl.h>
#define N(x) scenes_error_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

static const char* DISCORD_LINK = "discord.gg/QY8wzCMHyP";

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_title;
	C2D_Text g_errTitle;
	C2D_Text g_errSubtext;
	C2D_Text g_ifPersists;
	C2D_Text g_discordLink;
	C2D_Text g_exit;
	C2D_Image g_infoBox;
	C2D_Image g_qrCode;
	C2D_Image g_netpalDed;
} N(DataStruct);

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(1000);
	TextLangParse(&_data->g_title, _data->g_staticBuf, str_error_occurred);
	TextLangParse(&_data->g_ifPersists, _data->g_staticBuf, str_if_error_persists);
	C2D_TextParse(&_data->g_discordLink, _data->g_staticBuf, DISCORD_LINK);
	TextLangParse(&_data->g_exit, _data->g_staticBuf, str_press_to_exit);

	Result res = (Result)sc->data;
	char str[200];
	const char* subtext = "";
	C2D_Font str_font;
	C2D_Font subtext_font = _font(str_libcurl_error);
	if (res > -100) {
		// libcurl error code
		int errcode = -res;
		const char* errmsg = curl_easy_strerror(errcode);
		snprintf(str, 200, _s(str_libcurl_error), errcode, errmsg);
		str_font = _font(str_libcurl_error);
		if (errcode == 60) {
			subtext = _s(str_libcurl_date_and_time);
			subtext_font = _font(str_libcurl_date_and_time);
		}
	} else if (res > -600) {
		// http status code
		int status_code = -res;
		snprintf(str, 200, _s(str_httpstatus_error), status_code);
		str_font = _font(str_httpstatus_error);
	} else {
		// 3ds error code
		snprintf(str, 200, _s(str_3ds_error), (u32)res);
		str_font = _font(str_3ds_error);
	}
	C2D_TextFontParse(&_data->g_errTitle, str_font, _data->g_staticBuf, str);
	C2D_TextFontParse(&_data->g_errSubtext, subtext_font, _data->g_staticBuf, subtext);

	_data->g_infoBox = C2D_SpriteSheetGetImage(spr_misc, ui_misc_info_box);
	_data->g_qrCode = C2D_SpriteSheetGetImage(spr_misc, ui_misc_discord_qr);
	_data->g_netpalDed = C2D_SpriteSheetGetImage(spr_misc, ui_misc_netpal_ded);

	sc->setting.bg_top = bg_top_generic;
	sc->setting.bg_bottom = bg_bottom_generic;
	sc->setting.btn_left = ui_btn_empty;
	sc->setting.btn_right = ui_btn_empty;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;

	u16 imgWidth = _data->g_infoBox.subtex->width;
	u16 imgHeight = _data->g_infoBox.subtex->height;

	float x = CENTER_TOP_X(imgWidth);
	float y = CENTER_TOP_Y(imgHeight + 15);
	float z = 0;

	// Render box
	C2D_DrawImageAt(_data->g_infoBox, x, y, z, NULL, 1.0f, 1.0f);

	// Render text
	x += 15;
	C2D_DrawText(&_data->g_title, C2D_WithColor, x, y + 10, z, 0.85f, 0.85f, clr_white);

	u32 flags = C2D_WithColor | C2D_WordWrap;
	float maxWidth = imgWidth - x;
	C2D_DrawText(&_data->g_errTitle, flags, x, y + 40, z, 0.5f, 0.5f, clr_white, maxWidth);
	// TODO: calculate total height of error title and place error subtitle appropriately
	C2D_DrawText(&_data->g_errSubtext, flags, x, y + 75, z, 0.5f, 0.5f, clr_white, maxWidth);

	C2D_DrawText(&_data->g_ifPersists, flags, x, y + imgHeight - 45, z, 0.6f, 0.6f, clr_white, maxWidth);
}

void N(render_bottom)(Scene* sc) {
	if (!_data) return;

	float x = 15;
	float y = 15;
	float z = 0;

	// Render Discord link
	renderTextWithOutline(
		&_data->g_discordLink, 0,
		x, y, z,
		0.65f, 0.65f, 1.65f,
		clr_white, clr_netpass_green
	);

	// Render QR code
	u16 qrHeight = _data->g_qrCode.subtex->height * 5;
	u16 qrWidth = _data->g_qrCode.subtex->width * 5;
	C2D_DrawImageAt(_data->g_qrCode, x, CENTER_BOTTOM_Y(qrHeight), z, NULL, 5.0f, 5.0f);

	// Render ded NetPal
	C2D_DrawImageAt(_data->g_netpalDed, x + qrWidth + 25, 92, z, NULL, 1.0f, 1.0f);

	// Render exit text
	float txtHeight;
	get_text_dimensions(&_data->g_exit, 0.65f, 0.65f, NULL, &txtHeight);
	renderTextWithOutline(
		&_data->g_exit, 0,
		x, SCREEN_BOTTOM_HEIGHT - txtHeight - 15, z,
		0.65f, 0.65f, 1.65f,
		clr_white, clr_netpass_green
	);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	app_state = app_idle;
	return scene_continue;
}

Scene* getConnectionErrorScene(Result res) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render_top = N(render_top);
	scene->render_bottom = N(render_bottom);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = (u32)res;
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}