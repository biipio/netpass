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
#include "3ds.h"
#include "stdlib.h"

#define FRAMES_PER_SEC 16
#define MS_PER_FRAME (1000 / FRAMES_PER_SEC)

void netpalInit() {
}

void netpalExit() {
}

NetPal* buildNetPal(float x, float y, float z) {
	NetPal* pal = malloc(sizeof(NetPal));
	
	pal->x = x;
	pal->y = y;
	pal->z = z;

	pal->frame = 0;
	pal->loops = 0;
	pal->start = svcGetSystemTick() / CPU_TICKS_PER_MSEC;

	return pal;
}

void destroyNetPal(NetPal* pal) {
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
