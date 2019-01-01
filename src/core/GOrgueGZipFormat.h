/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GORGUEGZIPFORMAT_H
#define GORGUEGZIPFORMAT_H

#include "GOrgueInt.h"

#pragma pack(push, 1)

typedef struct GOGZipHeader
{
	GOUInt16BE signature;
	GOUInt8 method;
	GOUInt8 flags;
	GOUInt32 mtime;
	GOUInt8 xflags;
	GOUInt8 OS;
} GOGZipHeader;

typedef struct GOGZipTrailer
{
	GOUInt32LE crc;
	GOUInt32LE orig_size;
} GOGZipTrailer;

static constexpr uint16_t GZIP_SIGNATURE = 0x1f8b;
static constexpr uint8_t GZIP_METHOD = 8;

#pragma pack(pop)

#endif
