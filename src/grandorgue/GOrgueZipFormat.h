/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include <stdint.h>

#pragma pack(push, 1)

#define ZIP_LOCAL_HEADER             0x04034b50
#define ZIP_DATA_DESCRIPTOR          0x08074b50
#define ZIP_CENTRAL_DIRECTORY_HEADER 0x02014b50
#define ZIP_END_RECORD               0x06054b50
#define ZIP_END64_RECORD             0x06064b50
#define ZIP_END64_LOCATOR            0x07064b50

typedef struct GOZipLocalHeader
{
	uint32_t signature;
	uint16_t version_extract;
	uint16_t flags;
	uint16_t compression;
	uint16_t modification_time;
	uint16_t modification_date;
	uint32_t crc;
	uint32_t compressed_size;
	uint32_t uncompressed_size;
	uint16_t name_length;
	uint16_t extra_length;
	void Swap();
} GOZipLocalHeader;

typedef struct GOZipCentralHeader
{
	uint32_t signature;
	uint16_t version_creator;
	uint16_t version_extract;
	uint16_t flags;
	uint16_t compression;
	uint16_t modification_time;
	uint16_t modification_date;
	uint32_t crc;
	uint32_t compressed_size;
	uint32_t uncompressed_size;
	uint16_t name_length;
	uint16_t extra_length;
	uint16_t comment_length;
	uint16_t disk_number;
	uint16_t internal_attributes;
	uint32_t external_attributes;
	uint32_t offset;
	void Swap();
} GOZipCentralHeader;

typedef struct GOZipHeaderExtraRecord
{
	uint16_t type;
	uint16_t size;
	void Swap();
} GOZipHeaderExtraRecord;

typedef struct GOZipEnd64Record
{
	uint32_t signature;
	uint64_t size;
	uint16_t version_creator;
	uint16_t version_extract;
	uint32_t current_disk;
	uint32_t directory_disk;
	uint64_t entry_count_disk;
	uint64_t entry_count;
	uint64_t directory_size;
	uint64_t directory_offset;
	void Swap();
} GOZipEnd64Record;

typedef struct GOZipEnd64Locator
{
	uint32_t signature;
	uint32_t end_record_disk;
	uint64_t end_record_offset;
	uint32_t disk_count;
	void Swap();
} GOZipEnd64Locator;

typedef struct GOZipEnd64BlockHeader
{
	uint16_t header_id;
	uint32_t size;
	void Swap();
} GOZipEnd64BlockHeader;

typedef struct GOZipEndRecord
{
	uint32_t signature;
	uint16_t current_disk;
	uint16_t directory_disk;
	uint16_t entry_count_disk;
	uint16_t entry_count;
	uint32_t directory_size;
	uint32_t directory_offset;
	uint16_t comment_len;
	void Swap();
} GOZipEndRecord;

#pragma pack(pop)

#endif
