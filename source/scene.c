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
#include "render.h"
#include <malloc.h>
#include <3ds.h>

#define FADE_ALPHA_VAL 6
#define FADE_ALPHA_LIMIT (255 - (255 % FADE_ALPHA_VAL))

u32 app_state = app_opening; // init to app_opening

void updateInputState(Scene* scene) {
	hidScanInput();
	scene->input_state.k_down = hidKeysDown();
	scene->input_state.k_down_repeat = hidKeysDownRepeat();
	scene->input_state.k_held = hidKeysHeld();
	scene->input_state.k_up = hidKeysUp();
	scene->input_state.pos_prev = scene->input_state.pos_current;
	hidTouchRead(&scene->input_state.pos_current);
}

void updateListCursor(int* cursor, InputState* state, int cursorMax) {
	*cursor += (state->k_down_repeat & KEY_DOWN && 1) - (state->k_down_repeat & KEY_UP && 1);
	*cursor += (state->k_down_repeat & KEY_RIGHT && 1)*4 - (state->k_down_repeat & KEY_LEFT && 1)*4;
	if (state->k_down & (KEY_DOWN | KEY_UP)) {
		if (*cursor < 0) *cursor = cursorMax;
		if (*cursor > cursorMax) *cursor = 0;
	} else if (state->k_down_repeat & (KEY_DOWN | KEY_UP | KEY_RIGHT | KEY_LEFT)) {
		if (*cursor < 0) *cursor = 0;
		if (*cursor > cursorMax) *cursor = cursorMax;
	}
}

void updateListOffset(float* offset, int cursor) {
	if (cursor >= 0) {
		// TODO: treat as pixel, not list index
		if (cursor > *offset + 3) *offset = cursor - 3;
		if (cursor < *offset) *offset = cursor;
	}
}

Scene* processScene(Scene* scene) {
	updateInputState(scene);

	// Fade in
	if (app_state & app_opening) {
		if (fade_alpha <= FADE_ALPHA_VAL) {
			app_state ^= app_opening; // toggle off app_opening
			fade_alpha = 0;
		} else {
			fade_alpha -= FADE_ALPHA_VAL;
		}
	}

	// Fade out
	if (app_state & app_exiting) {
		if ((fade_alpha >= FADE_ALPHA_LIMIT) || (scene->input_state.k_down_repeat & KEY_START)) {
			scene->exit(scene);
			if (scene->need_free) {
				free(scene);
			}
			return 0;
		}
		fade_alpha += FADE_ALPHA_VAL;
		return scene;
	}

	if (app_state & (app_idle | app_netpal_speaking)) {
		if (scene->input_state.k_down_repeat & KEY_START) {
			hidSetRepeatParameters(18, 12);
			app_state = app_exiting | (app_state & app_netpal_speaking);
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