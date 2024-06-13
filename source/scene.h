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
typedef enum {app_loading, app_idle, app_exiting} AppState;

typedef struct Scene Scene;

typedef struct {
	u32 k_down;
	u32 k_down_repeat;
	u32 k_held;
	u32 k_up;
	touchPosition pos_prev;
	touchPosition pos_current;
} InputState;

typedef struct {
	int bg_top;
	int bg_bottom;
	int btn_left;
	int btn_right;
	bool has_gradient;
	bool use_previews;
	u8 fade_alpha;
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
	AppState app_state;
	Setting setting;

	void* d;
	bool is_popup;
	bool need_free;
};

Scene* processScene(Scene* scene);

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