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

void updateInputState(InputState* state) {
	hidScanInput();

	state->k_down = hidKeysDown();
	state->k_down_repeat = hidKeysDownRepeat();
	state->k_held = hidKeysHeld();
	state->k_up = hidKeysUp();

	state->pos_prev = state->pos_current;
	hidTouchRead(&state->pos_current);
	if (state->k_down & KEY_TOUCH) {
		state->isTouched = true;
		state->pos_start = state->pos_current;
		state->pos_prev = state->pos_current;
	}
	if (state->k_up & KEY_TOUCH) {
		state->isTouched = false;
	}
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

void updateListOffset(float* offset, float* velocity, int* cursor, int btnCount, InputState* state) {
	// Define constants
	const float SCROLL_SLOW_RATE = 0.85f;
	const float RUBBERBAND_SLOW_RATE = 0.5f;
	const float MIN_OFFSET = 0;
	const float MAX_OFFSET = 45 + (35 * btnCount) + (10 * (btnCount - 1)) + 30 - SCREEN_BOTTOM_HEIGHT;

	// Calculate distance scrolled this frame
	int scroll_distance = state->pos_prev.py - state->pos_current.py;
	
	// Calculate how far above the max offset or below the min offset the current offset is
	float unsafe_offset_distance = 0;
	if (*offset < MIN_OFFSET) unsafe_offset_distance = (MIN_OFFSET - *offset); // positive number
	if (*offset > MAX_OFFSET) unsafe_offset_distance = (MAX_OFFSET - *offset); // negative number

	if (state->k_down & KEY_TOUCH) {
		// Screen was just tapped; reset velocity and cursor
		*velocity = 0;
		*cursor = -1;
	} else if (state->isTouched) {
		// Currently scrolling
		if (unsafe_offset_distance != 0) {
			// Scrolling unsafely, so scroll slowly
			if (unsafe_offset_distance > 5) {
				*offset += scroll_distance * (5 / unsafe_offset_distance);
			} else if (unsafe_offset_distance < -5) {
				*offset += scroll_distance * (-5 / unsafe_offset_distance);
			} else {
				*offset += scroll_distance;
			}
			*velocity = RUBBERBAND_SLOW_RATE * unsafe_offset_distance;
		} else {
			// Scroll normally
			*velocity = scroll_distance;
			*offset += *velocity;
		}
	} else {
		// Currently not scrolling
		if (unsafe_offset_distance != 0) {
			*velocity = RUBBERBAND_SLOW_RATE * unsafe_offset_distance;
		} else {
			*velocity *= SCROLL_SLOW_RATE;
		}

		// Cursor scrolling takes priority
		if (*cursor > -1) {
			float btn_offset = *cursor * 45; // cursor * (height of one button + height of gap between buttons)
			if (btn_offset < *offset) {
				// Hovered button is off-screen up
				*velocity = RUBBERBAND_SLOW_RATE * (btn_offset - *offset);
			}
			if (btn_offset + 35 > *offset + SCREEN_BOTTOM_HEIGHT - 75) {
				// Hovered button is off-screen down
				*velocity = RUBBERBAND_SLOW_RATE * ((btn_offset + 35) - (*offset + SCREEN_BOTTOM_HEIGHT - 75));
			}
		}

		// Apply velocity to offset
		*offset += *velocity;

		// Set velocity to 0 if it's insanely close to 0
		if ((*velocity > 0 && *velocity < 1) || (*velocity < 0 && *velocity > -1)) {
			*velocity = 0;
		}
	}
}

Scene* processScene(Scene* scene) {
	updateInputState(&scene->input_state);

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