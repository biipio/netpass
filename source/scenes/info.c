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
#include "../special_chars.h"
#include <stdlib.h>
#define N(x) scenes_scene_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

#define MARGIN 20
#define WIDTH (SCREEN_TOP_WIDTH - 2*MARGIN)
#define HEIGHT (SCREEN_TOP_HEIGHT - 2*MARGIN)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_info;
	C2D_Text g_aButton;
	C2D_Image g_infoBox;
} N(DataStruct);

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	TextLangParse(&_data->g_info, _data->g_staticBuf, (void*)sc->data);
	C2D_TextParse(&_data->g_aButton, _data->g_staticBuf, STR_BUTTON_A);
	_data->g_infoBox = C2D_SpriteSheetGetImage(spr_misc, ui_misc_info_box);
	
	Setting popSetting = sc->pop_scene->setting;
	sc->setting.fade_alpha = 0;
	sc->setting.bg_top = popSetting.bg_top;
	sc->setting.bg_bottom = popSetting.bg_bottom;
	sc->setting.btn_left = popSetting.btn_left;
	sc->setting.btn_right = popSetting.btn_right;
	sc->setting.has_gradient = popSetting.has_gradient;
	sc->setting.use_previews = popSetting.use_previews;
}

void N(render_top)(Scene* sc) {
	if (!_data) return;
	
	float x = CENTER_TOP_X(368);
	float y = CENTER_TOP_Y(182*0.6);
	float z = 0;
	
	// Render box
	C2D_DrawImageAt(_data->g_infoBox, x, y, z, NULL, 1.0f, 0.6f);
	u16 imgWidth = _data->g_infoBox.subtex->width;
	u16 imgHeight = (_data->g_infoBox.subtex->height) * 0.6f;

	// Render text
	u32 flags = C2D_AlignCenter | C2D_WithColor | C2D_WordWrap;
	C2D_DrawText(&_data->g_info, flags, x + imgWidth/2, y + 10, z, 0.75f, 0.75f, clr_white, imgWidth - (x + 10)*2);
	
	// Render A button
	C2D_DrawText(&_data->g_aButton, C2D_WithColor, x + imgWidth - 25, y + imgHeight - 25, z, 0.75f, 0.75f, clr_white);
}

void N(render_bottom)(Scene* sc) { }

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	app_state = app_idle;
	if (sc->input_state.k_down & (KEY_A | KEY_B | KEY_TOUCH)) return scene_pop;
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