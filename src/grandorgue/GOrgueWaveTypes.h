/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GORGUEWAVETYPES_H
#define GORGUEWAVETYPES_H

#include <wx/wx.h>

#pragma pack(push, 1)

typedef char GO_FOURCC[4];

typedef struct
{
	GO_FOURCC fccChunk;
	wxUint32 dwSize;
} GO_WAVECHUNKHEADER;

typedef struct
{
	wxUint16 wFormatTag;
	wxUint16 nChannels;
	wxUint32 nSamplesPerSec;
	wxUint32 nAvgBytesPerSec;
	wxUint16 nBlockAlign;
} GO_WAVEFORMAT;

typedef struct
{
	GO_WAVEFORMAT wf;
	wxUint16 wBitsPerSample;
} GO_WAVEFORMATPCM;

typedef struct
{
	GO_WAVEFORMATPCM wf;
	wxUint16 cbSize;
} GO_WAVEFORMATPCMEX;

typedef struct
{
	wxUint32 dwName;
	wxUint32 dwPosition;
	GO_FOURCC fccChunk;
	wxUint32 dwChunkStart;
	wxUint32 dwBlockStart;
	wxUint32 dwSampleOffset;
} GO_WAVECUEPOINT;

typedef struct
{
	wxUint32 dwCuePoints;
} GO_WAVECUECHUNK;

typedef struct
{
	wxUint32 dwIdentifier;
	wxUint32 dwType;
	wxUint32 dwStart;
	wxUint32 dwEnd;
	wxUint32 dwFraction;
	wxUint32 dwPlayCount;
} GO_WAVESAMPLERLOOP;

typedef struct
{
	wxUint32 dwManufacturer;
	wxUint32 dwProduct;
	wxUint32 dwSamplePeriod;
	wxUint32 dwMIDIUnityNote;
	wxUint32 dwMIDIPitchFraction;
	wxUint32 dwSMPTEFormat;
	wxUint32 dwSMPTEOffset;
	wxUint32 cSampleLoops;
	wxUint32 cbSamplerData;
} GO_WAVESAMPLERCHUNK;

bool inline CompareFourCC(GO_FOURCC fcc, const char* text)
{
	assert(text != NULL);
	assert(strlen(text) == 4);
	return ((fcc[0] == text[0]) &&
			(fcc[1] == text[1]) &&
			(fcc[2] == text[2]) &&
			(fcc[3] == text[3]));
}

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
