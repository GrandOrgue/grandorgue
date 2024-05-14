/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOWAVETYPES_H
#define GOWAVETYPES_H

#include "GOInt.h"

#pragma pack(push, 1)

static constexpr uint32_t WAVEChunk(const char name[4]) {
  return (name[0] << 24) | (name[1] << 16) | (name[2] << 8) | (name[3] << 0);
}
static constexpr uint32_t WAVE_TYPE_RIFF = WAVEChunk("RIFF");
static constexpr uint32_t WAVE_TYPE_WAVE = WAVEChunk("WAVE");
static constexpr uint32_t WAVE_TYPE_DATA = WAVEChunk("data");
static constexpr uint32_t WAVE_TYPE_FMT = WAVEChunk("fmt ");
static constexpr uint32_t WAVE_TYPE_CUE = WAVEChunk("cue ");
static constexpr uint32_t WAVE_TYPE_SAMPLE = WAVEChunk("smpl");

typedef GOUInt32BE GO_WAVETYPEFIELD;

typedef struct {
  GO_WAVETYPEFIELD fccChunk;
  GOUInt32LE dwSize;
} GO_WAVECHUNKHEADER;

typedef struct {
  GOUInt16LE wFormatTag;
  GOUInt16LE nChannels;
  GOUInt32LE nSamplesPerSec;
  GOUInt32LE nAvgBytesPerSec;
  GOUInt16LE nBlockAlign;
} GO_WAVEFORMAT;

typedef struct {
  GO_WAVEFORMAT wf;
  GOUInt16LE wBitsPerSample;
} GO_WAVEFORMATPCM;

typedef struct {
  GO_WAVEFORMATPCM wf;
  GOUInt16LE cbSize;
} GO_WAVEFORMATPCMEX;

typedef struct {
  GOUInt32LE dwName;
  GOUInt32LE dwPosition;
  GO_WAVETYPEFIELD fccChunk;
  GOUInt32LE dwChunkStart;
  GOUInt32LE dwBlockStart;
  GOUInt32LE dwSampleOffset;
} GO_WAVECUEPOINT;

typedef struct {
  GOUInt32LE dwCuePoints;
} GO_WAVECUECHUNK;

typedef struct {
  GOUInt32LE dwIdentifier;
  GOUInt32LE dwType;
  GOUInt32LE dwStart;
  GOUInt32LE dwEnd;
  GOUInt32LE dwFraction;
  GOUInt32LE dwPlayCount;
} GO_WAVESAMPLERLOOP;

typedef struct {
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

#endif /* GOWAVETYPES_H */
