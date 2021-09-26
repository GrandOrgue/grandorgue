/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECOMPRESS_H
#define GORGUECOMPRESS_H

#include <stdint.h>

template<class T>
class GOrgueBuffer;

bool compressBuffer(GOrgueBuffer<uint8_t>& buffer);

bool isBufferCompressed(const GOrgueBuffer<uint8_t>& buffer);

bool uncompressBuffer(GOrgueBuffer<uint8_t>& buffer);

#endif
