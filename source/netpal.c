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

#include "netpal.h"
#include <3ds.h>
#include <stdlib.h>

#define FRAMES_PER_SEC 16
#define MS_PER_FRAME (1000 / FRAMES_PER_SEC)

static char* path_leaf[] = {
	"romfs:/gfx/netpal_leaf_neutral.t3x",
	"romfs:/gfx/netpal_leaf_neutral_outline.t3x",
	"romfs:/gfx/netpal_leaf_shake.t3x",
	"romfs:/gfx/netpal_leaf_shake_outline.t3x",
};
static char* path_head[] = {
	"romfs:/gfx/netpal_head.t3x",
	"romfs:/gfx/netpal_head_outline.t3x",
};

static char* path_eye_left[] = {
	"romfs:/gfx/netpal_eye_left_open.t3x",
	"romfs:/gfx/netpal_eye_left_closed.t3x",
	"romfs:/gfx/netpal_eye_left_happy.t3x",
	"romfs:/gfx/netpal_eye_left_sad.t3x",
	"romfs:/gfx/netpal_eye_left_surprised.t3x",
	"romfs:/gfx/netpal_eye_left_squint.t3x",
	"romfs:/gfx/netpal_eye_left_dizzy.t3x",
};
static char* path_eye_right[] = {
	"romfs:/gfx/netpal_eye_right_open.t3x",
	"romfs:/gfx/netpal_eye_right_closed.t3x",
	"romfs:/gfx/netpal_eye_right_happy.t3x",
	"romfs:/gfx/netpal_eye_right_sad.t3x",
	"romfs:/gfx/netpal_eye_right_surprised.t3x",
	"romfs:/gfx/netpal_eye_right_squint.t3x",
	"romfs:/gfx/netpal_eye_right_dizzy.t3x",
};
static char* path_mouth[] = {
	"romfs:/gfx/netpal_mouth_neutral.t3x",
	"romfs:/gfx/netpal_mouth_smile.t3x",
	"romfs:/gfx/netpal_mouth_frown.t3x",
	"romfs:/gfx/netpal_mouth_happy.t3x",
	"romfs:/gfx/netpal_mouth_happy_talk.t3x",
	"romfs:/gfx/netpal_mouth_unhappy_talk.t3x",
	"romfs:/gfx/netpal_mouth_surprise.t3x",
	"romfs:/gfx/netpal_mouth_nervous.t3x",
	"romfs:/gfx/netpal_mouth_devastated.t3x",
};

static char* path_hand_left[] = {
	"romfs:/gfx/netpal_hand_left_neutral.t3x",
	"romfs:/gfx/netpal_hand_left_neutral_outline.t3x",
	"romfs:/gfx/netpal_hand_left_wave.t3x",
	"romfs:/gfx/netpal_hand_left_wave_outline.t3x",
};
static char* path_hand_right[] = {
	"romfs:/gfx/netpal_hand_right_neutral.t3x",
	"romfs:/gfx/netpal_hand_right_neutral_outline.t3x",
	"romfs:/gfx/netpal_hand_right_wave.t3x",
	"romfs:/gfx/netpal_hand_right_wave_outline.t3x",
};

static char* path_foot_left[] = {
	"romfs:/gfx/netpal_foot_left_neutral.t3x",
	"romfs:/gfx/netpal_foot_left_stomp.t3x",
};
static char* path_foot_right[] = {
	"romfs:/gfx/netpal_foot_right_neutral.t3x",
	"romfs:/gfx/netpal_foot_right_stomp.t3x",
};

struct _NetPalSheet  {
	int loaded_state;
	C2D_SpriteSheet spr;
	C2D_SpriteSheet spr_outline;
};

struct _NetPalSheets {
	struct _NetPalSheet leaf;
	struct _NetPalSheet head;
	struct _NetPalSheet eye_left; // no outline
	struct _NetPalSheet eye_right; // no outline
	struct _NetPalSheet mouth; // no outline
	struct _NetPalSheet hand_left;
	struct _NetPalSheet hand_right;
	struct _NetPalSheet foot_left; // no outline
	struct _NetPalSheet foot_right; // no outline
};

NetPal* buildNetPal(float x, float y, float z) {
	NetPal* pal = malloc(sizeof(NetPal));

	pal->x = x;
	pal->y = y;
	pal->z = z;

	pal->frame = 0;
	pal->loops = 0;
	pal->start = svcGetSystemTick() / CPU_TICKS_PER_MSEC;

	pal->state_leaf = NETPAL_LEAF_NEUTRAL;
	pal->state_head = NETPAL_HEAD_NEUTRAL;

	pal->state_eye_left = NETPAL_EYE_OPEN;
	pal->state_eye_right = NETPAL_EYE_OPEN;
	pal->state_mouth = NETPAL_MOUTH_NEUTRAL;

	pal->state_hand_left = NETPAL_HAND_NEUTRAL;
	pal->state_hand_right = NETPAL_HAND_NEUTRAL;

	pal->state_foot_left = NETPAL_FOOT_NEUTRAL;
	pal->state_foot_right = NETPAL_FOOT_NEUTRAL;

	pal->sheets = malloc(sizeof(struct _NetPalSheets));

	pal->sheets->leaf = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->head = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->eye_left = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->eye_right = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->mouth = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->hand_left = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->hand_right = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->foot_left = (struct _NetPalSheet) {-1, NULL, NULL};
	pal->sheets->foot_right = (struct _NetPalSheet) {-1, NULL, NULL};

	return pal;
}

static inline void freeSpriteSheet(C2D_SpriteSheet spr) {
	if (spr != NULL) C2D_SpriteSheetFree(spr);
}

void destroyNetPal(NetPal* pal) {
	freeSpriteSheet(pal->sheets->leaf.spr);
	freeSpriteSheet(pal->sheets->leaf.spr_outline);
	freeSpriteSheet(pal->sheets->head.spr);
	freeSpriteSheet(pal->sheets->head.spr_outline);

	freeSpriteSheet(pal->sheets->eye_left.spr);
	freeSpriteSheet(pal->sheets->eye_right.spr);
	freeSpriteSheet(pal->sheets->mouth.spr);

	freeSpriteSheet(pal->sheets->hand_left.spr);
	freeSpriteSheet(pal->sheets->hand_left.spr_outline);
	freeSpriteSheet(pal->sheets->hand_right.spr);
	freeSpriteSheet(pal->sheets->hand_right.spr_outline);

	freeSpriteSheet(pal->sheets->foot_left.spr);
	freeSpriteSheet(pal->sheets->foot_right.spr);

	free(pal->sheets);

	free(pal);
}

void updateFrame(NetPal* pal) {
	u64 elapsedMs = (svcGetSystemTick() / CPU_TICKS_PER_MSEC) - pal->start;

	if (elapsedMs >= MS_PER_FRAME) {
		pal->frame++;
		if (pal->frame > 31) {
			pal->frame = 0;
			pal->loops++;
		}

		pal->start = svcGetSystemTick() / CPU_TICKS_PER_MSEC;
	}
}

static inline void loadSpriteSheet(C2D_SpriteSheet* spr, const char* path) {
	// NOTE: This frees a spritesheet containing a currently-displayed sprite. Unfortunately there's not enough
	//       memory(?) to load the new spritesheet, render the new sprite, then free the old spritesheet
	if (*spr != NULL) C2D_SpriteSheetFree(*spr);
	*spr = C2D_SpriteSheetLoad(path);
}

static inline void renderPart(NetPal* pal, C2D_SpriteSheet spr) {
	if (spr == NULL) return;
	C2D_Image img = C2D_SpriteSheetGetImage(spr, pal->frame);
	C2D_DrawImageAt(img, pal->x, pal->y, pal->z, NULL, 1.0f, 1.0f);
}

void renderNetPal(NetPal* pal) {
	// Free/Load spritesheets if necessary
	if (pal->state_leaf != pal->sheets->leaf.loaded_state) {
		loadSpriteSheet(&pal->sheets->leaf.spr, path_leaf[pal->state_leaf*2]);
		loadSpriteSheet(&pal->sheets->leaf.spr_outline, path_leaf[(pal->state_leaf*2) + 1]);
		pal->sheets->leaf.loaded_state = pal->state_leaf;
	}
	if (pal->state_head != pal->sheets->head.loaded_state) {
		loadSpriteSheet(&pal->sheets->head.spr, path_head[pal->state_head*2]);
		loadSpriteSheet(&pal->sheets->head.spr_outline, path_head[(pal->state_head*2) + 1]);
		pal->sheets->head.loaded_state = pal->state_head;
	}

	if (pal->state_eye_left != pal->sheets->eye_left.loaded_state) {
		loadSpriteSheet(&pal->sheets->eye_left.spr, path_eye_left[pal->state_eye_left]);
		pal->sheets->eye_left.loaded_state = pal->state_eye_left;
	}
	if (pal->state_eye_right != pal->sheets->eye_right.loaded_state) {
		loadSpriteSheet(&pal->sheets->eye_right.spr, path_eye_right[pal->state_eye_right]);
		pal->sheets->eye_right.loaded_state = pal->state_eye_right;
	}
	if (pal->state_mouth != pal->sheets->mouth.loaded_state) {
		loadSpriteSheet(&pal->sheets->mouth.spr, path_mouth[pal->state_mouth]);
		pal->sheets->mouth.loaded_state = pal->state_mouth;
	}

	if (pal->state_hand_left != pal->sheets->hand_left.loaded_state) {
		loadSpriteSheet(&pal->sheets->hand_left.spr, path_hand_left[pal->state_hand_left*2]);
		loadSpriteSheet(&pal->sheets->hand_left.spr_outline, path_hand_left[(pal->state_hand_left*2) + 1]);
		pal->sheets->hand_left.loaded_state = pal->state_hand_left;
	}
	if (pal->state_hand_right != pal->sheets->hand_right.loaded_state) {
		loadSpriteSheet(&pal->sheets->hand_right.spr, path_hand_right[pal->state_hand_right*2]);
		loadSpriteSheet(&pal->sheets->hand_right.spr_outline, path_hand_right[(pal->state_hand_right*2) + 1]);
		pal->sheets->hand_right.loaded_state = pal->state_hand_right;
	}

	if (pal->state_foot_left != pal->sheets->foot_left.loaded_state) {
		loadSpriteSheet(&pal->sheets->foot_left.spr, path_foot_left[pal->state_foot_left]);
		pal->sheets->foot_left.loaded_state = pal->state_foot_left;
	}
	if (pal->state_foot_right != pal->sheets->foot_right.loaded_state) {
		loadSpriteSheet(&pal->sheets->foot_right.spr, path_foot_right[pal->state_foot_right]);
		pal->sheets->foot_right.loaded_state = pal->state_foot_right;
	}

	// Parts must be rendered in this order

	renderPart(pal, pal->sheets->head.spr_outline);
	renderPart(pal, pal->sheets->leaf.spr_outline);
	renderPart(pal, pal->sheets->hand_left.spr_outline);
	renderPart(pal, pal->sheets->hand_right.spr_outline);
	renderPart(pal, pal->sheets->foot_left.spr);
	renderPart(pal, pal->sheets->foot_right.spr);
	renderPart(pal, pal->sheets->head.spr);
	renderPart(pal, pal->sheets->hand_left.spr);
	renderPart(pal, pal->sheets->hand_right.spr);
	renderPart(pal, pal->sheets->leaf.spr);
	renderPart(pal, pal->sheets->eye_left.spr);
	renderPart(pal, pal->sheets->eye_right.spr);
	renderPart(pal, pal->sheets->mouth.spr);
}
