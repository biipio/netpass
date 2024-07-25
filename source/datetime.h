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

typedef struct {
	u32 year;
	u8 month;    // 1 - 12
	u8 day;      // 1 - 31
	u8 weekday;  // 0 - 6
	u8 hour;     // 0 - 23
	u8 minute;   // 0 - 59
	u8 second;   // 0 - 59
} DateTime;

void getRtcTime(DateTime* dt);
void getSystemTime(DateTime* dt);
