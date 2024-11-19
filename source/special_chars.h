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

// https://www.3dbrew.org/wiki/System_Font#Unicode_Private_Use_characters
#define STR_BUTTON_A            "\uE000"
#define STR_BUTTON_B            "\uE001"
#define STR_BUTTON_X            "\uE002"
#define STR_BUTTON_Y            "\uE003"
#define STR_BUTTON_L            "\uE004"
#define STR_BUTTON_R            "\uE005"

#define STR_DPAD                "\uE006"
#define STR_DPAD_UP             "\uE079"
#define STR_DPAD_DOWN           "\uE07A"
#define STR_DPAD_LEFT           "\uE07B"
#define STR_DPAD_RIGHT          "\uE07C"
#define STR_DPAD_VERTICAL       "\uE07D"
#define STR_DPAD_HORIZONTAL     "\uE07E"

#define STR_ANALOG_STICK        "\uE077"
#define STR_POWER_BUTTON        "\uE078"

#define STR_ICON_BACK           "\uE072"
#define STR_ICON_HOME           "\uE073"
#define STR_ICON_STEPS          "\uE074"
#define STR_ICON_PLAYCOIN       "\uE075"
#define STR_ICON_VIDEO_CAMERA   "\uE076"

#define STR_SPINNER_UP_LEFT     "\uE020"
#define STR_SPINNER_UP          "\uE021"
#define STR_SPINNER_UP_RIGHT    "\uE022"
#define STR_SPINNER_RIGHT       "\uE023"
#define STR_SPINNER_DOWN_RIGHT  "\uE024"
#define STR_SPINNER_DOWN        "\uE025"
#define STR_SPINNER_DOWN_LEFT   "\uE026"
#define STR_SPINNER_LEFT        "\uE027"

const char* STR_SPINNER[] = {
	STR_SPINNER_UP_LEFT,
	STR_SPINNER_UP,
	STR_SPINNER_UP_RIGHT,
	STR_SPINNER_RIGHT,
	STR_SPINNER_DOWN_RIGHT,
	STR_SPINNER_DOWN,
	STR_SPINNER_DOWN_LEFT,
	STR_SPINNER_LEFT
};
