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

#include "scene.h"
#include <malloc.h>
#include <3ds.h>

#define FADE_ALPHA_VAL 4
#define FADE_ALPHA_LIMIT (255 - (255 % FADE_ALPHA_VAL))

void updateInputState(Scene* scene) {
	hidScanInput();
	scene->input_state.k_down = hidKeysDown();
	scene->input_state.k_down_repeat = hidKeysDownRepeat();
	scene->input_state.k_held = hidKeysHeld();
	scene->input_state.k_up = hidKeysUp();
	scene->input_state.pos_prev = scene->input_state.pos_current;
	hidTouchRead(&scene->input_state.pos_current);
}

Scene* processScene(Scene* scene) {
	if (scene->app_state == app_exiting) {
		updateInputState(scene);
		if ((scene->setting.fade_alpha >= FADE_ALPHA_LIMIT) || (scene->input_state.k_down_repeat & KEY_START)) {
			scene->exit(scene);
			if (scene->need_free) {
				free(scene);
			}
			return 0;
		}
		scene->setting.fade_alpha += FADE_ALPHA_VAL;
		return scene;
	}

	if (scene->app_state == app_idle) {
		updateInputState(scene);
		if (scene->input_state.k_down & KEY_START) {
			hidSetRepeatParameters(18, 12);
			scene->app_state = app_exiting;
			return scene;
		}
	}
	SceneResult res = scene->process(scene);
	switch (res) {
	case scene_continue:
	{
		return scene;
	}
	case scene_stop:
	{
		scene->exit(scene);
		if (scene->need_free) {
			free(scene);
		}
		return 0;
	}
	case scene_switch:
	{
		Scene* new_scene = scene->next_scene;
		if (!new_scene) {
			printf("ERROR: Could not create scene!!");
			return NULL;
		}
		scene->exit(scene);
		if (scene->need_free) {
			free(scene);
		}
		new_scene->init(new_scene);
		return new_scene;
	}
	case scene_push:
	{
		Scene* new_scene = scene->next_scene;
		new_scene->pop_scene = scene;
		new_scene->init(new_scene);
		return new_scene;
	}
	case scene_pop:
	{
		Scene* new_scene = scene->pop_scene;
		scene->exit(scene);
		if (scene->need_free) {
			free(scene);
		}
		return new_scene;
	}
	}
	return scene;
}