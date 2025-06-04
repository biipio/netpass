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

typedef struct BossHTTPHeader {
	char name[0x20];
	char value[0x100];
} BossHTTPHeader;

typedef enum BossPropertyId {
	BOSSPROPERTY_DURATION = 0x04,
	BOSSPROPERTY_URL = 0x07,
	BOSSPROPERTY_HTTPHEADERS = 0x0D,
	BOSSPROPERTY_TOTALTASKS = 0x35,
	BOSSPROPERTY_TASKIDS = 0x36,
} BossPropertyId;

typedef BossHTTPHeader* BossHTTPHeaders;

Result bossUnregisterTask(char* task_id, u16 step_id);