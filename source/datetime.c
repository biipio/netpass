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

#include "datetime.h"
#include <time.h>

#define BCD_TO_BINARY(__byte) ((10 * ((__byte) >> 4)) + ((__byte) & 0xF))

static inline u8 getWeekdayFromDate(u8 d, u8 m, u32 y) {
	// source: https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Keith
	return (d+=m<3 ? y-- : y-2,23*m/9+d+4+y/4-y/100+y/400) % 7;
}

void getRtcTime(DateTime* dt) {
	// see https://www.3dbrew.org/wiki/I2C_Registers#Device_3
	u64 rtc;
	MCUHWC_ReadRegister(0x30, &rtc, 7);

	dt->second = BCD_TO_BINARY(rtc & 0xFF);
	dt->minute = BCD_TO_BINARY((rtc >> 1*8) & 0xFF);
	dt->hour = BCD_TO_BINARY((rtc >> 2*8) & 0xFF);
	dt->day = BCD_TO_BINARY((rtc >> 4*8) & 0xFF);
	dt->month = BCD_TO_BINARY((rtc >> 5*8) & 0xFF);
	dt->year = BCD_TO_BINARY((rtc >> 6*8) & 0xFF) + 2000;
	dt->weekday = getWeekdayFromDate(dt->day, dt->month, dt->year);
}

void getSystemTime(DateTime* dt) {
	time_t unixTime = time(NULL);
	struct tm* tm = gmtime(&unixTime);

	dt->second = tm->tm_sec;
	dt->minute = tm->tm_min;
	dt->hour = tm->tm_hour;
	dt->day = tm->tm_mday;
	dt->month = tm->tm_mon + 1;
	dt->year = tm->tm_year + 1900;
	dt->weekday = tm->tm_wday;
}
