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

#include <3ds.h>
#include <citro2d.h>

typedef enum {
	NETPAL_LEAF_NEUTRAL,
	NETPAL_LEAF_SHAKE
} NetPalLeafState;

typedef enum {
	NETPAL_HEAD_NEUTRAL
} NetPalHeadState;

typedef enum {
	NETPAL_EYE_OPEN,
	NETPAL_EYE_CLOSED,
	NETPAL_EYE_HAPPY,
	NETPAL_EYE_SAD,
	NETPAL_EYE_SURPRISED,
	NETPAL_EYE_SQUINT,
	NETPAL_EYE_DIZZY
} NetPalEyeState;

typedef enum {
	NETPAL_MOUTH_NEUTRAL,
	NETPAL_MOUTH_SMILE,
	NETPAL_MOUTH_FROWN,
	NETPAL_MOUTH_HAPPY,
	NETPAL_MOUTH_HAPPY_TALK,
	NETPAL_MOUTH_UNHAPPY_TALK,
	NETPAL_MOUTH_SURPRISE,
	NETPAL_MOUTH_NERVOUS,
	NETPAL_MOUTH_DEVASTATED
} NetPalMouthState;

typedef enum {
	NETPAL_HAND_NEUTRAL,
	NETPAL_HAND_WAVE
} NetPalHandState;

typedef enum {
	NETPAL_FOOT_NEUTRAL,
	NETPAL_FOOT_STOMP
} NetPalFootState;

typedef struct _NetPalSheets* NetPalSheets;
typedef struct NetPal {
	float x;
	float y;
	float z;

	unsigned int frame;
	unsigned int loops;
	u64 start;

	NetPalLeafState state_leaf;
	NetPalHeadState state_head;

	NetPalEyeState state_eye_left;
	NetPalEyeState state_eye_right;
	NetPalMouthState state_mouth;

	NetPalHandState state_hand_left;
	NetPalHandState state_hand_right;
	NetPalFootState state_foot_left;
	NetPalFootState state_foot_right;

	NetPalSheets sheets;
} NetPal;

NetPal* buildNetPal(float x, float y, float z);
void destroyNetPal(NetPal* pal);

void updateFrame(NetPal* pal);
void renderNetPal(NetPal* pal);

static inline void netpalIdleBlink(NetPal* pal) {
	bool doAction = (pal->loops % 2) == 1 && (pal->frame > 29);
	pal->state_eye_left = doAction ? NETPAL_EYE_CLOSED : NETPAL_EYE_OPEN;
	pal->state_eye_right = doAction ? NETPAL_EYE_CLOSED : NETPAL_EYE_OPEN;
}

static inline void netpalIdleLeafShake(NetPal* pal) {
	bool doAction = (pal->loops % 6) == 3;
	pal->state_leaf = doAction ? NETPAL_LEAF_SHAKE : NETPAL_LEAF_NEUTRAL;
}
