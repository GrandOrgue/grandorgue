/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIFILE_H
#define GORGUEMIDIFILE_H

#include "GOrgueInt.h"

#pragma pack(push, 1)

typedef struct {
	char type[4];
	GOUInt32BE len;
} MIDIFileHeader;

typedef struct {
	MIDIFileHeader header;
	GOUInt16BE type;
	GOUInt16BE tracks;
	GOUInt16BE ppq;
} MIDIHeaderChunk;

#pragma pack(pop)

#endif
