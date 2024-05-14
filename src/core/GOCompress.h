/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMPRESS_H
#define GOCOMPRESS_H

#include <stdint.h>

template <class T> class GOBuffer;

bool compressBuffer(GOBuffer<uint8_t> &buffer);

bool isBufferCompressed(const GOBuffer<uint8_t> &buffer);

bool uncompressBuffer(GOBuffer<uint8_t> &buffer);

#endif
