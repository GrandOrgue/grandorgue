/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include <stdint.h>

#pragma pack(push, 1)

typedef char GO_FOURCC[4];

typedef struct
{
	GO_FOURCC fccChunk;
	uint32_t dwSize;
} GO_WAVECHUNKHEADER;

typedef struct
{
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
} GO_WAVEFORMAT;

typedef struct
{
	GO_WAVEFORMAT wf;
	uint16_t wBitsPerSample;
} GO_WAVEFORMATPCM;

typedef struct
{
	GO_WAVEFORMATPCM wf;
	uint16_t cbSize;
} GO_WAVEFORMATPCMEX;

typedef struct
{
	uint32_t dwName;
	uint32_t dwPosition;
	GO_FOURCC fccChunk;
	uint32_t dwChunkStart;
	uint32_t dwBlockStart;
	uint32_t dwSampleOffset;
} GO_WAVECUEPOINT;

typedef struct
{
	uint32_t dwCuePoints;
} GO_WAVECUECHUNK;

typedef struct
{
	uint32_t dwIdentifier;
	uint32_t dwType;
	uint32_t dwStart;
	uint32_t dwEnd;
	uint32_t dwFraction;
	uint32_t dwPlayCount;
} GO_WAVESAMPLERLOOP;

typedef struct
{
	uint32_t dwManufacturer;
	uint32_t dwProduct;
	uint32_t dwSamplePeriod;
	uint32_t dwMIDIUnityNote;
	uint32_t dwMIDIPitchFraction;
	uint32_t dwSMPTEFormat;
	uint32_t dwSMPTEOffset;
	uint32_t cSampleLoops;
	uint32_t cbSamplerData;
} GO_WAVESAMPLERCHUNK;

typedef struct {
	unsigned char lo, mi;
	signed char hi;
} GO_Int24;

inline GO_Int24 IntToGOInt24(int value)
{
	GO_Int24 val;
	val.lo = (value >> 0) & 0xff;
	val.mi = (value >> 8) & 0xff;
	val.hi = (value >> 16) & 0xff;
	return val;
}

inline int GOInt24ToInt(GO_Int24 value)
{
	return ((value.hi << 16) | (value.mi << 8) | value.lo);
}

#pragma pack(pop)

#endif /* GORGUEWAVETYPES_H */
