/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEWAVETYPES_H
#define GORGUEWAVETYPES_H

#include "GOrgueInt.h"

#pragma pack(push, 1)

typedef char GO_FOURCC[4];

typedef struct
{
	GO_FOURCC fccChunk;
	GOUInt32LE dwSize;
} GO_WAVECHUNKHEADER;

typedef struct
{
	GOUInt16LE wFormatTag;
	GOUInt16LE nChannels;
	GOUInt32LE nSamplesPerSec;
	GOUInt32LE nAvgBytesPerSec;
	GOUInt16LE nBlockAlign;
} GO_WAVEFORMAT;

typedef struct
{
	GO_WAVEFORMAT wf;
	GOUInt16LE wBitsPerSample;
} GO_WAVEFORMATPCM;

typedef struct
{
	GO_WAVEFORMATPCM wf;
	GOUInt16LE cbSize;
} GO_WAVEFORMATPCMEX;

typedef struct
{
	GOUInt32LE dwName;
	GOUInt32LE dwPosition;
	GO_FOURCC fccChunk;
	GOUInt32LE dwChunkStart;
	GOUInt32LE dwBlockStart;
	GOUInt32LE dwSampleOffset;
} GO_WAVECUEPOINT;

typedef struct
{
	GOUInt32LE dwCuePoints;
} GO_WAVECUECHUNK;

typedef struct
{
	GOUInt32LE dwIdentifier;
	GOUInt32LE dwType;
	GOUInt32LE dwStart;
	GOUInt32LE dwEnd;
	GOUInt32LE dwFraction;
	GOUInt32LE dwPlayCount;
} GO_WAVESAMPLERLOOP;

typedef struct
{
	GOUInt32LE dwManufacturer;
	GOUInt32LE dwProduct;
	GOUInt32LE dwSamplePeriod;
	GOUInt32LE dwMIDIUnityNote;
	GOUInt32LE dwMIDIPitchFraction;
	GOUInt32LE dwSMPTEFormat;
	GOUInt32LE dwSMPTEOffset;
	GOUInt32LE cSampleLoops;
	GOUInt32LE cbSamplerData;
} GO_WAVESAMPLERCHUNK;

#pragma pack(pop)

#endif /* GORGUEWAVETYPES_H */
