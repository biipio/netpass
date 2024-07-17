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

#pragma once

#include <citro2d.h>
#include <3ds.h>
#include "scene.h"

#define SCREEN_TOP_WIDTH 400
#define SCREEN_TOP_HEIGHT 240

#define SCREEN_BOTTOM_WIDTH 320
#define SCREEN_BOTTOM_HEIGHT 240

#define CENTER_TOP_X(img_width) ((SCREEN_TOP_WIDTH - img_width) / 2)
#define CENTER_TOP_Y(img_height) ((SCREEN_TOP_HEIGHT - img_height) / 2)

#define CENTER_BOTTOM_X(img_width) ((SCREEN_BOTTOM_WIDTH - img_width) / 2)
#define CENTER_BOTTOM_Y(img_height) ((SCREEN_BOTTOM_HEIGHT - img_height) / 2)

typedef enum {
	bg_top_generic,
	bg_top_home,
	bg_top_train_station,
	bg_top_plaza,
	bg_top_mall,
	bg_top_beach,
	bg_top_arcade,
	bg_top_cat_cafe
} BackgroundTop;
typedef enum {
	bg_bottom_generic
} BackgroundBottom;

typedef enum {
	ui_btn_empty = -1,
	ui_btn_text,
	ui_btn_text_selected,
	ui_btn_left_help,
	ui_btn_left_home,
	ui_btn_left_back_alley,
	ui_btn_left_close,
	ui_btn_right_settings,
	ui_btn_right_close
} UI_Button;

typedef enum {
	ui_misc_logo,
	ui_misc_gradient,
	ui_misc_bar,
	ui_misc_upper_arrow,
	ui_misc_lower_arrow,
	ui_misc_info_box,
	ui_misc_speech_bubble
} UI_Misc;

extern C2D_SpriteSheet spr_misc;

extern u32 clr_white;
extern u32 clr_gray;
extern u32 clr_black;
extern u32 clr_netpass_green;
extern u32 clr_focus_blue;

extern u8 fade_alpha;

void renderInit(void);
void renderExit(void);

void renderImage(C2D_SpriteSheet spr, size_t index, float x, float y, float z);
void renderOptionButtons(C2D_Text* entries, size_t n, int cursor, float offset, int grayedOut);
void renderTextWithOutline(C2D_Text* text, u32 flags, float x, float y, float z, float scaleX, float scaleY, float outlineWidth, u32 textClr, u32 outlineClr);

void renderScene(Scene* scene);

#include "netpal.h"
