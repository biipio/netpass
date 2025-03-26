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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "render.h"
#include "strings.h"
#include "config.h"
#include "datetime.h"

static const size_t path_bg_top_count = 14;
static const char* path_bg_top[] = {
	"romfs:/gfx/bg_top_generic.t3x",
	"romfs:/gfx/bg_top_home.t3x",
	"romfs:/gfx/bg_top_train_station.t3x",
	"romfs:/gfx/bg_top_train_station_preview.t3x",
	"romfs:/gfx/bg_top_plaza.t3x",
	"romfs:/gfx/bg_top_plaza_preview.t3x",
	"romfs:/gfx/bg_top_mall.t3x",
	"romfs:/gfx/bg_top_mall_preview.t3x",
	"romfs:/gfx/bg_top_beach.t3x",
	"romfs:/gfx/bg_top_beach_preview.t3x",
	"romfs:/gfx/bg_top_arcade.t3x",
	"romfs:/gfx/bg_top_arcade_preview.t3x",
	"romfs:/gfx/bg_top_cat_cafe.t3x",
	"romfs:/gfx/bg_top_cat_cafe_preview.t3x"
};

u8 fade_alpha = 255; // init to 255 for fade in upon opening

static C2D_TextBuf g_dynamicBuf;

static C3D_RenderTarget* top;
static C3D_RenderTarget* bottom;

struct _SpriteSheet {
	u8 index;
	C2D_SpriteSheet sheet;
} _SpriteSheet;

static struct _SpriteSheet sheet_bg_top;
static C2D_SpriteSheet spr_bg_bottom;
static C2D_SpriteSheet spr_btn;
static C2D_SpriteSheet spr_wifi;
static C2D_SpriteSheet spr_battery;
C2D_SpriteSheet spr_misc;

static C2D_Image img_btn_text;

u32 clr_white;
u32 clr_gray;
u32 clr_black;
u32 clr_netpass_green;
u32 clr_focus_blue;
u32 clr_off_red;

void renderInit(void) {
	g_dynamicBuf = C2D_TextBufNew(2000);

	top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	sheet_bg_top = (struct _SpriteSheet) {-1, NULL};
	spr_bg_bottom = C2D_SpriteSheetLoad("romfs:/gfx/bg_bottom.t3x");
	spr_btn = C2D_SpriteSheetLoad("romfs:/gfx/button.t3x");
	spr_wifi = C2D_SpriteSheetLoad("romfs:/gfx/wifi.t3x");
	spr_battery = C2D_SpriteSheetLoad("romfs:/gfx/battery.t3x");
	spr_misc = C2D_SpriteSheetLoad("romfs:/gfx/misc.t3x");

	img_btn_text = C2D_SpriteSheetGetImage(spr_btn, ui_btn_text);

	clr_white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
	clr_gray = C2D_Color32(0x90, 0x90, 0x90, 0xFF);
	clr_black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
	clr_netpass_green = C2D_Color32(0x1F, 0xAF, 0x5F, 0xFF);
	clr_focus_blue = C2D_Color32(0x08, 0xB4, 0xC4, 0xFF);
	clr_off_red = C2D_Color32(0xC8, 0x0A, 0x0A, 0xFF);
}

void renderExit(void) {
	C2D_TextBufDelete(g_dynamicBuf);
	
	if (sheet_bg_top.sheet != NULL) C2D_SpriteSheetFree(sheet_bg_top.sheet);
	C2D_SpriteSheetFree(spr_bg_bottom);
	C2D_SpriteSheetFree(spr_btn);
	C2D_SpriteSheetFree(spr_wifi);
	C2D_SpriteSheetFree(spr_battery);
	C2D_SpriteSheetFree(spr_misc);
}

void renderImage(C2D_SpriteSheet spr, size_t index, float x, float y, float z) {
	if (index < 0 || index > C2D_SpriteSheetCount(spr)) return;
	C2D_Image img = C2D_SpriteSheetGetImage(spr, index);
	C2D_DrawImageAt(img, x, y, z, NULL, 1.0f, 1.0f);
}

void renderOptionButton(C2D_Text* text, float x, float y, float z, u32 clr, bool isFocus) {
	// Define constants
	u32 clr_default = clr_netpass_green;

	// Render option button with proper tint if button color is not default
	if (clr != clr_default) {
		C2D_ImageTint tint;
		C2D_PlainImageTint(&tint, clr, 0.25f);
		C2D_DrawImageAt(img_btn_text, x, y, z, &tint, 1, 1);
	} else {
		C2D_DrawImageAt(img_btn_text, x, y, z, NULL, 1, 1);
	}

	// Render glowing blue outline if the current button is focused
	if (isFocus) {
		renderImage(spr_btn, ui_btn_text_selected, x - 5, y - 4, z);
	}

	// Scale input text based on option button image dimensions
	u16 button_width = img_btn_text.subtex->width;
	u16 button_height = img_btn_text.subtex->height;
	float scale_x = 0.8f;
	float scale_y = 0.8f;
	get_scale_for_fit(text, button_width - 10, button_height - 4, &scale_x, &scale_y);
	
	// Calculate correct x and y for text to be centered within the button
	float text_height;
	get_text_dimensions(text, scale_x, scale_y, NULL, &text_height);
	float text_x = x + button_width/2;
	float text_y = y + (button_height - text_height)/2;

	// Determine text and text outline colors
	u32 clr_text = clr_white;
	u32 clr_outline = clr;
	if (isFocus && clr == clr_default) {
		clr_outline = clr_focus_blue;
	}

	// Render the text
	renderTextWithOutline(text, C2D_AlignCenter, text_x, text_y, z, scale_x, scale_y, 1.6f, clr_text, clr_outline);
}

void renderOptionButtons(C2D_Text* entries, Setting* setting) {
	// Define constants
	const int x = CENTER_BOTTOM_X(258);
	const int z = 0;
	const int gap = 10;
	const int btnHeight = 35;

	// Get values from setting
	const float offset = setting->scroll_offset;
	const float cursor = setting->btn_cursor;
	const int n = setting->btn_count;

	// Calculate starting y
	int totalHeight = (btnHeight * n) + (gap * (n - 1));
	int y = CENTER_BOTTOM_Y(totalHeight);

	// Render any visible buttons
	for (size_t i = 0; i < n; i++) {
		int btnY = 45 + (i * (btnHeight + gap));
		if (offset + SCREEN_BOTTOM_HEIGHT < btnY) continue;
		if (offset > btnY + btnHeight) continue;

		bool isFocus = (i == cursor);
		if (n < 4) {
			// If there are less than 4 buttons total, center them on the screen
			renderOptionButton(&entries[i], x, y + (i*(btnHeight+gap)), z, setting->btn_colors[i], isFocus);
		} else {
			// If there are at least 4 buttons total, adjust their height with offset
			renderOptionButton(&entries[i], x, btnY - offset, z, setting->btn_colors[i], isFocus);
		}
	}

	// Display top/bottom arrows if any buttons are even slightly off-screen
	if (n > 4) {
		if (offset > 45) {
			renderImage(spr_misc, ui_misc_upper_arrow, 0, 0, 0);
		}
		if (offset + SCREEN_BOTTOM_HEIGHT < 45 + totalHeight) {
			renderImage(spr_misc, ui_misc_lower_arrow, 0, SCREEN_BOTTOM_HEIGHT - 40, 0);
		}
	}
}

void renderTextWithOutline(C2D_Text* text, u32 flags, float x, float y, float z, float scaleX, float scaleY, float outlineWidth, u32 textClr, u32 outlineClr, ...) {
	// I hate this so much this is so stupid

	float xPos = x + outlineWidth;
	float xNeg = x - outlineWidth;
	float yPos = y + outlineWidth;
	float yNeg = y - outlineWidth;

	va_list args;
	va_start(args, outlineClr);

	// Outline
	C2D_DrawText(text, C2D_WithColor | flags, xPos, y,    z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, xPos, yPos, z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, x,    yPos, z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, xNeg, yPos, z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, xNeg, y,    z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, xNeg, yNeg, z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, x,    yNeg, z, scaleX, scaleY, outlineClr, args);
	C2D_DrawText(text, C2D_WithColor | flags, xPos, yNeg, z, scaleX, scaleY, outlineClr, args);

	// Actual text
	C2D_DrawText(text, C2D_WithColor | flags, x, y, z, scaleX, scaleY, textClr, args);

	va_end(args);
}

void resetBtnColors(u32** btn_colors, size_t n, u32 clr) {
	if (*btn_colors != NULL) free(*btn_colors);

	*btn_colors = malloc(n * sizeof(u32));
	for (size_t i = 0; i < n; i++) {
		(*btn_colors)[i] = clr;
	}
}

// void renderUser(ReportListEntry* user) {
// 	const int x = (SCREEN_TOP_WIDTH - 368)/2;
// 	const int y = (SCREEN_TOP_HEIGHT + 21 - 183)/2;

// 	renderImage(spr_misc, ui_misc_info_box, x, y, 0);

// 	char miiName[11] = {0};
// 	utf16_to_utf8((u8*)miiName, user->mii.mii_name, 11);
// 	C2D_Text txt_miiName;
// 	C2D_TextFontParse(&txt_miiName, getFontIndex(user->mii.mii_options.char_set), g_dynamicBuf, miiName);
// 	C2D_DrawText(&txt_miiName, C2D_WithColor, x + 10, y + 10, 0.5f, 0.8f, 0.8f, clr_white);

// 	const char* s = _s(str_last_met);
// 	C2D_Font font = _font(str_last_met);
// 	char text[50];
// 	snprintf(text, 50, s, user->received.year, user->received.month, user->received.day,
// 			user->received.hour, user->received.minute, user->received.second);
// 	C2D_Text txt_lastMet;
// 	C2D_TextFontParse(&txt_lastMet, font, g_dynamicBuf, text);
// 	C2D_DrawText(&txt_lastMet, C2D_WithColor, x + 10, y + 30, 0.5f, 0.8f, 0.8f, clr_white);
// }

void renderTopBar() {
	static LanguageString* months[12] = {
		&str_january,
		&str_february,
		&str_march,
		&str_april,
		&str_may,
		&str_june,
		&str_july,
		&str_august,
		&str_september,
		&str_october,
		&str_november,
		&str_december
	};
	static LanguageString* weekdays[7] = {
		&str_sunday,
		&str_monday,
		&str_tuesday,
		&str_wednesday,
		&str_thursday,
		&str_friday,
		&str_saturday
	};

	const float y = 2;
	const float z = 0;

	renderImage(spr_misc, ui_misc_bar, 0, 0, 0);

	DateTime dt;
	getSystemTime(&dt);

	// Date
	const char* monthStr = _s(*months[dt.month - 1]);
	const char* weekdayStr = _s(*weekdays[dt.weekday]);
	char dateStr[25];
	snprintf(dateStr, 25, "%s, %s %d", weekdayStr, monthStr, dt.day);

	C2D_Text dateText;
	C2D_TextParse(&dateText, g_dynamicBuf, dateStr);
	C2D_DrawText(&dateText, C2D_WithColor, 3, y, z, 0.5f, 0.5f, clr_white);

	// Time
	char timeStr[9];
	switch (config.time_format) {
	case 0: // 24 hour
		snprintf(timeStr, 6, "%02u:%02u", dt.hour%25, dt.minute%61);
		break;
	case 1: // 12 hour
		char* format = dt.hour > 12 ? "%2u:%02u PM" : "%2u:%02u AM";
		snprintf(timeStr, 9, format, ((dt.hour + 11) % 12) + 1, dt.minute);
		break;
	}

	C2D_Text timeText;
	C2D_TextParse(&timeText, g_dynamicBuf, timeStr);
	C2D_DrawText(&timeText, C2D_AlignCenter | C2D_WithColor, SCREEN_TOP_WIDTH / 2, y, z, 0.5f, 0.5f, clr_white);

	// Wifi
	renderImage(spr_wifi, osGetWifiStrength(), SCREEN_TOP_WIDTH - 57, y, z);

	// Battery
	u8 index;
	u8 batteryIsCharging;
	PTMU_GetBatteryChargeState(&batteryIsCharging);
	if (batteryIsCharging) {
		index = 5;
	} else {
		PTMU_GetBatteryLevel(&index);
		if (index > 0) index--;
	}
	renderImage(spr_battery, index, SCREEN_TOP_WIDTH - 30, y, z);
}

void renderTopScreen(Scene* scene) {
	Setting setting = scene->setting;
	
	// Render background
	{
		// If setting.bg_top doesn't exist, default to bg_top_generic
		if ((path_bg_top_count / 2) + 1 < setting.bg_top) {
			setting.bg_top = bg_top_generic;
		}

		u8 index = setting.bg_top;
		if (setting.bg_top > bg_top_home)
			index += setting.use_previews ? setting.bg_top - 1 : setting.bg_top - 2;

		if (sheet_bg_top.index != index) {
			// Render new background before freeing old background
			// ^ (necessary because freeing the currently displayed background has weird results)
			sheet_bg_top.index = index;
			C2D_SpriteSheet temp_spr = C2D_SpriteSheetLoad(path_bg_top[sheet_bg_top.index]);
			renderImage(temp_spr, 0, 0, 0, 0);

			if (sheet_bg_top.sheet != NULL) C2D_SpriteSheetFree(sheet_bg_top.sheet);
			sheet_bg_top.sheet = temp_spr;
		} else {
			renderImage(sheet_bg_top.sheet, 0, 0, 0, 0);
		}
	}

	// Render gradient if necessary
	if (setting.bg_top > bg_top_generic) {
		renderImage(spr_misc, ui_misc_gradient, 0, SCREEN_TOP_HEIGHT - 45, 0);
	}

	// Let the scene render itself
	if (scene->is_popup) {
		scene->pop_scene->render_top(scene->pop_scene);
		C2D_Flush();
	}
	scene->render_top(scene);
	
	// Render top bar
	renderTopBar();

	// Render fade if necessary
	if (app_state & (app_opening | app_exiting)) {
		u32 clr = C2D_Color32(0, 0, 0, fade_alpha);
		C2D_DrawRectSolid(0, 0, 0, SCREEN_TOP_WIDTH, SCREEN_TOP_HEIGHT, clr);
	}
}

void renderBottomScreen(Scene* scene) {
	Setting setting = scene->setting;

	// Render background
	if (C2D_SpriteSheetCount(spr_bg_bottom) <= setting.bg_bottom) {
		setting.bg_bottom = bg_bottom_generic;
	}
	renderImage(spr_bg_bottom, setting.bg_bottom, 0, 0, 0);

	// Let the scene render itself
	if (scene->is_popup) {
		scene->pop_scene->render_bottom(scene->pop_scene);
		C2D_Flush();
	}
	scene->render_bottom(scene);

	// Render left and right buttons
	renderImage(spr_btn, setting.btn_left, 0, 0, 0);
	renderImage(spr_btn, setting.btn_right, SCREEN_BOTTOM_WIDTH - 49, 0, 0);

	// Render fade if necessary
	if (app_state & (app_opening | app_exiting)) {
		u32 clr = C2D_Color32(0, 0, 0, fade_alpha);
		C2D_DrawRectSolid(0, 0, 0, SCREEN_BOTTOM_WIDTH, SCREEN_BOTTOM_HEIGHT, clr);
	}
}

void renderScene(Scene* scene) {
	C2D_TextBufClear(g_dynamicBuf);
	
	C2D_TargetClear(top, clr_white);
	C2D_SceneBegin(top);
	renderTopScreen(scene);
	
	C2D_TargetClear(bottom, clr_white);
	C2D_SceneBegin(bottom);
	renderBottomScreen(scene);
}
