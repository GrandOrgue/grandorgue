/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEZIPFORMAT_H
#define GORGUEZIPFORMAT_H

#include "GOrgueInt.h"

#pragma pack(push, 1)

#define ZIP_LOCAL_HEADER             0x04034b50
#define ZIP_DATA_DESCRIPTOR          0x08074b50
#define ZIP_CENTRAL_DIRECTORY_HEADER 0x02014b50
#define ZIP_END_RECORD               0x06054b50
#define ZIP_END64_RECORD             0x06064b50
#define ZIP_END64_LOCATOR            0x07064b50

typedef struct GOZipLocalHeader
{
	GOUInt32LE signature;
	GOUInt16LE version_extract;
	GOUInt16LE flags;
	GOUInt16LE compression;
	GOUInt16LE modification_time;
	GOUInt16LE modification_date;
	GOUInt32LE crc;
	GOUInt32LE compressed_size;
	GOUInt32LE uncompressed_size;
	GOUInt16LE name_length;
	GOUInt16LE extra_length;
} GOZipLocalHeader;

typedef struct GOZipCentralHeader
{
	GOUInt32LE signature;
	GOUInt16LE version_creator;
	GOUInt16LE version_extract;
	GOUInt16LE flags;
	GOUInt16LE compression;
	GOUInt16LE modification_time;
	GOUInt16LE modification_date;
	GOUInt32LE crc;
	GOUInt32LE compressed_size;
	GOUInt32LE uncompressed_size;
	GOUInt16LE name_length;
	GOUInt16LE extra_length;
	GOUInt16LE comment_length;
	GOUInt16LE disk_number;
	GOUInt16LE internal_attributes;
	GOUInt32LE external_attributes;
	GOUInt32LE offset;
} GOZipCentralHeader;

typedef struct GOZipHeaderExtraRecord
{
	GOUInt16LE type;
	GOUInt16LE size;
} GOZipHeaderExtraRecord;

typedef struct GOZipEnd64Record
{
	GOUInt32LE signature;
	GOUInt64LE size;
	GOUInt16LE version_creator;
	GOUInt16LE version_extract;
	GOUInt32LE current_disk;
	GOUInt32LE directory_disk;
	GOUInt64LE entry_count_disk;
	GOUInt64LE entry_count;
	GOUInt64LE directory_size;
	GOUInt64LE directory_offset;
} GOZipEnd64Record;

typedef struct GOZipEnd64Locator
{
	GOUInt32LE signature;
	GOUInt32LE end_record_disk;
	GOUInt64LE end_record_offset;
	GOUInt32LE disk_count;
} GOZipEnd64Locator;

typedef struct GOZipEnd64BlockHeader
{
	GOUInt16LE header_id;
	GOUInt32LE size;
} GOZipEnd64BlockHeader;

typedef struct GOZipEndRecord
{
	GOUInt32LE signature;
	GOUInt16LE current_disk;
	GOUInt16LE directory_disk;
	GOUInt16LE entry_count_disk;
	GOUInt16LE entry_count;
	GOUInt32LE directory_size;
	GOUInt32LE directory_offset;
	GOUInt16LE comment_len;
} GOZipEndRecord;

#pragma pack(pop)

#endif
