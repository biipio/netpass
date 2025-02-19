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

#pragma once

#include <3ds.h>
#include <citro2d.h>
#include "api.h"
#include "strings.h"
#include "config.h"

typedef enum {scene_stop, scene_continue, scene_switch, scene_push, scene_pop} SceneResult;
typedef enum {
	app_loading = BIT(0),
	app_idle = BIT(1),
	app_netpal_speaking = BIT(2),
	app_opening = BIT(3),
	app_exiting = BIT(4)
} AppState;

typedef struct Scene Scene;

typedef struct {
	u32 k_down;                // Buttons that are newly pressed this frame
	u32 k_down_repeat;         // Buttons that are either newly pressed or held this frame
	u32 k_held;                // Buttons that are held this frame
	u32 k_up;                  // Buttons that are newly release this frame
	
	bool isTouched;            // Whether the touch screen is currently being touched 
	touchPosition pos_start;   // Position being touched in the frame the screen was first touched
	touchPosition pos_prev;    // Position being touched in the previous frame
	touchPosition pos_current; // Position being touched in the current frame
} InputState;

typedef struct {
	int bg_top;
	int bg_bottom;
	int btn_left;
	int btn_right;
	bool has_gradient;
	bool use_previews;
} Setting;

struct Scene {
	void (*init)(Scene*);
	void (*render_top)(Scene*);
	void (*render_bottom)(Scene*);
	void (*exit)(Scene*);
	SceneResult (*process)(Scene*);
	Scene* next_scene;
	Scene* pop_scene;
	u32 data;
	InputState input_state;
	Setting setting;

	void* d;
	bool is_popup;
	bool need_free;
};

void updateListCursor(int* cursor, InputState* state, int cursorMax);
void updateListOffset(float* offset, int cursor);
Scene* processScene(Scene* scene);

extern u32 app_state;

#include "scenes/loading.h"
#include "scenes/switch.h"
#include "scenes/home.h"
#include "scenes/location.h"
#include "scenes/connection_error.h"
#include "scenes/settings.h"
#include "scenes/info.h"
#include "scenes/report_list.h"
#include "scenes/back_alley.h"
#include "scenes/toggle_titles.h"