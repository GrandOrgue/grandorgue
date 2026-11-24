/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestWave.h"

#include "GOBuffer.h"
#include "GOWave.h"
#include "GOWaveTypes.h"

#include "GOTestScope.h"
#include "GOTestUtils.h"

const std::string GOTestWave::TEST_NAME = "GOTestWave";

/**
 * @param isFloat whether the items are IEEE floats (format code 3) or PCM (1)
 * @param nBits the bits per item: 8, 16, 24 or 32
 * @param pSamples the items of the only channel, already in the file layout
 * @param nSamples the number of items; the data must have an even size
 * @return a RIFF/WAVE file with one channel at 48 kHz
 */
static GOBuffer<uint8_t> make_wav(
  bool isFloat, unsigned nBits, const void *pSamples, unsigned nSamples) {
  const unsigned nDataBytes = nSamples * nBits / 8;
  const GO_WAVECHUNKHEADER riff
    = {WAVE_TYPE_RIFF, 4 + 8 + sizeof(GO_WAVEFORMATPCM) + 8 + nDataBytes};
  const GO_WAVETYPEFIELD wave = WAVE_TYPE_WAVE;
  const GO_WAVECHUNKHEADER fmtHead = {WAVE_TYPE_FMT, sizeof(GO_WAVEFORMATPCM)};
  GO_WAVEFORMATPCM fmt;
  const GO_WAVECHUNKHEADER dataHead = {WAVE_TYPE_DATA, nDataBytes};
  GOBuffer<uint8_t> wav;

  fmt.wf.wFormatTag = isFloat ? 3 : 1;
  fmt.wf.nChannels = 1;
  fmt.wf.nSamplesPerSec = 48000;
  fmt.wf.nAvgBytesPerSec = 48000 * nBits / 8;
  fmt.wf.nBlockAlign = nBits / 8;
  fmt.wBitsPerSample = nBits;
  wav.Append((const uint8_t *)&riff, sizeof(riff));
  wav.Append((const uint8_t *)&wave, sizeof(wave));
  wav.Append((const uint8_t *)&fmtHead, sizeof(fmtHead));
  wav.Append((const uint8_t *)&fmt, sizeof(fmt));
  wav.Append((const uint8_t *)&dataHead, sizeof(dataHead));
  wav.Append((const uint8_t *)pSamples, nDataBytes);
  return wav;
}

void GOTestWave::TestEightBitSigned() {
  // Offset binary: 0x80 is zero
  const uint8_t pcm[4] = {0x00, 0x80, 0xFF, 0x40};
  GOWave wave;

  wave.Open(make_wav(false, 8, pcm, 4), wxT("eight.wav"));

  int8_t bytes[4];

  wave.ReadSamples(bytes, GOWave::SF_SIGNEDBYTE_8, wave.GetSampleRate(), 1);
  GOAssert(
    bytes[0] == -128 && bytes[1] == 0 && bytes[2] == 127 && bytes[3] == -64,
    "8-bit samples must be decoded as signed");

  float floats[4];

  wave.ReadSamples(floats, GOWave::SF_IEEE_FLOAT, wave.GetSampleRate(), 1);
  GOAssert(
    floats[0] == -1.0f && floats[1] == 0.0f && floats[2] == 127.0f / 128.0f
      && floats[3] == -0.5f,
    "8-bit samples must scale to the exact float values");
}

void GOTestWave::run() { GO_RUN_TEST(TestEightBitSigned()) }
