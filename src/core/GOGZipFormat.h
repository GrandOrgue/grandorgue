/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGZIPFORMAT_H
#define GOGZIPFORMAT_H

#include "GOInt.h"

#pragma pack(push, 1)

typedef struct GOGZipHeader {
  GOUInt16BE signature;
  GOUInt8 method;
  GOUInt8 flags;
  GOUInt32 mtime;
  GOUInt8 xflags;
  GOUInt8 OS;
} GOGZipHeader;

typedef struct GOGZipTrailer {
  GOUInt32LE crc;
  GOUInt32LE orig_size;
} GOGZipTrailer;

static constexpr uint16_t GZIP_SIGNATURE = 0x1f8b;
static constexpr uint8_t GZIP_METHOD = 8;

#pragma pack(pop)

#endif
