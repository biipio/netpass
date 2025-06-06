/**
 * NetPass
 * Copyright (C) 2024-2025 Sorunome
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

#include "cecd.h"
#include "scene.h"
#include <3ds.h>
#include <3ds/types.h>
#include <citro2d.h>
#include <citro3d.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define _GET_MACRO_MAX_(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define MAX(...) _GET_MACRO_MAX_(__VA_ARGS__, MAX8, MAX7, MAX6, MAX5, MAX4, MAX3, MAX2, MAX1)(__VA_ARGS__)
#define MAX1(a) (a)
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) MAX2(a, MAX2(b, c))
#define MAX4(a, b, c, d) MAX2(a, MAX3(b, c, d))
#define MAX5(a, b, c, d, e) MAX2(a, MAX4(b, c, d, e))
#define MAX6(a, b, c, d, e, f) MAX2(a, MAX5(b, c, d, e, f))
#define MAX7(a, b, c, d, e, f, g) MAX2(a, MAX6(b, c, d, e, f, g))
#define MAX8(a, b, c, d, e, f, g, h) MAX2(a, MAX7(b, c, d, e, f, g, h))

void* cecGetExtHeader(CecMessageHeader* msg, u32 type);
u32 cecGetExtHeaderSize(CecMessageHeader* msg, u32 type);
char* b64encode(u8* in, size_t len);
int rmdir_r(char *path);
void mkdir_p(char* orig_path);
Result APT_Wrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out);
Result APT_Unwrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out);
u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val);
Result decryptMii(void* data, MiiData* mii);
u8* memsearch(u8* buf, size_t buf_len, u8* cmp, size_t cmp_len);
void C2D_ImageDelete(C2D_Image* img);
bool loadJpeg(C2D_Image* img, u8* data, u32 size);
size_t fread_blk(void* buffer, size_t size, size_t count, FILE* stream);
size_t fwrite_blk(void* buffer, size_t size, size_t nmemb, FILE* stream);
char* fgets_blk(char* str, int num, FILE* stream);
int fputs_blk(const char* str, FILE* stream);
void open_url(char* url);
Result get_os_version(OS_VersionBin* ver);
void _e(int error);
Scene* get_new_error_scene(void);

typedef struct {
	u32 magic; // 0x4F00
	u16 total_coins;
	u16 today_coins;
	u32 total_step_count_last_coin;
	u32 today_step_count_last_coin;
	u16 year;
	u8 month;
	u8 day;
} PlayCoins;
