/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestWave.h"

#include <cstring>

#include "GOBuffer.h"
#include "GOWave.h"
#include "GOWaveTypes.h"

#include "GOTestScope.h"
#include "GOTestUtils.h"

const std::string GOTestWave::TEST_NAME = "GOTestWave";

/**
 * @param isFloat whether the items are IEEE floats (format code 3) or PCM (1)
 * @param nBits the bits per item: 8, 16, 24 or 32
 * @param pSamples the interleaved items, already in the file layout
 * @param nSamples the number of items of all the channels; the data must have
 *   an even size
 * @param nChannels the number of channels
 * @return a RIFF/WAVE file at 48 kHz
 */
static GOBuffer<uint8_t> make_wav(
  bool isFloat,
  unsigned nBits,
  const void *pSamples,
  unsigned nSamples,
  unsigned nChannels = 1) {
  const unsigned nDataBytes = nSamples * nBits / 8;
  const GO_WAVECHUNKHEADER riff
    = {WAVE_TYPE_RIFF, 4 + 8 + sizeof(GO_WAVEFORMATPCM) + 8 + nDataBytes};
  const GO_WAVETYPEFIELD wave = WAVE_TYPE_WAVE;
  const GO_WAVECHUNKHEADER fmtHead = {WAVE_TYPE_FMT, sizeof(GO_WAVEFORMATPCM)};
  GO_WAVEFORMATPCM fmt;
  const GO_WAVECHUNKHEADER dataHead = {WAVE_TYPE_DATA, nDataBytes};
  GOBuffer<uint8_t> wav;

  fmt.wf.wFormatTag = isFloat ? 3 : 1;
  fmt.wf.nChannels = nChannels;
  fmt.wf.nSamplesPerSec = 48000;
  fmt.wf.nAvgBytesPerSec = 48000 * nBits / 8 * nChannels;
  fmt.wf.nBlockAlign = nBits / 8 * nChannels;
  fmt.wBitsPerSample = nBits;
  wav.Append((const uint8_t *)&riff, sizeof(riff));
  wav.Append((const uint8_t *)&wave, sizeof(wave));
  wav.Append((const uint8_t *)&fmtHead, sizeof(fmtHead));
  wav.Append((const uint8_t *)&fmt, sizeof(fmt));
  wav.Append((const uint8_t *)&dataHead, sizeof(dataHead));
  wav.Append((const uint8_t *)pSamples, nDataBytes);
  return wav;
}

void GOTestWave::TestIntegersUnchanged() {
  const int16_t pcm16[4] = {0x1234, -2, 32767, -32768};
  GOWave wave;

  wave.Open(make_wav(false, 16, pcm16, 4), wxT("sixteen.wav"));

  int16_t out16[4];

  wave.ReadSamples(out16, GOWave::SF_SIGNEDSHORT_16, wave.GetSampleRate(), 1);
  GOAssert(
    memcmp(out16, pcm16, sizeof(pcm16)) == 0,
    "16-bit samples must come out unchanged");

  // Little-endian 24-bit items: 0x123456, -2, +FS, -FS
  const uint8_t pcm24[12]
    = {0x56, 0x34, 0x12, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x80};

  wave.Open(make_wav(false, 24, pcm24, 4), wxT("twentyfour.wav"));

  uint8_t out24[12];

  wave.ReadSamples(out24, GOWave::SF_SIGNEDINT24_24, wave.GetSampleRate(), 1);
  GOAssert(
    memcmp(out24, pcm24, sizeof(pcm24)) == 0,
    "24-bit samples must come out unchanged");
}

void GOTestWave::TestFloatBitExact() {
  const float floatData[4] = {0.1f, -0.3f, 1.0f, 1e-7f};
  GOWave wave;

  wave.Open(make_wav(true, 32, floatData, 4), wxT("float.wav"));
  GOAssert(wave.IsFloat(), "a float file must be recognised as float");

  float out[4];

  wave.ReadSamples(out, GOWave::SF_IEEE_FLOAT, wave.GetSampleRate(), 1);
  GOAssert(
    memcmp(out, floatData, sizeof(floatData)) == 0,
    "float samples must stay bit-exact");
}

void GOTestWave::TestFloatChannelSelectionAndDownmix() {
  // two frames of {left, right}: small values and values beyond full scale
  const float stereo[4] = {1e-8f, 2e-8f, 3.0f, -1.5f};
  GOWave wave;

  wave.Open(make_wav(true, 32, stereo, 4, 2), wxT("stereo-float.wav"));

  float mono[2];

  wave.ReadSamples(mono, GOWave::SF_IEEE_FLOAT, wave.GetSampleRate(), 1);
  GOAssert(
    mono[0] == (1e-8f + 2e-8f) / 2 && mono[1] == (3.0f + -1.5f) / 2,
    "a float downmix must be computed in float");
  wave.ReadSamples(mono, GOWave::SF_IEEE_FLOAT, wave.GetSampleRate(), -1);
  GOAssert(
    mono[0] == 1e-8f && mono[1] == 3.0f,
    "the left channel of a float file must stay bit-exact");
  wave.ReadSamples(mono, GOWave::SF_IEEE_FLOAT, wave.GetSampleRate(), -2);
  GOAssert(
    mono[0] == 2e-8f && mono[1] == -1.5f,
    "the right channel of a float file must stay bit-exact");
}

void GOTestWave::TestThirtyTwoBitPcmRejected() {
  const int32_t pcm32[2] = {0x12345678, -1};
  GOWave wave;
  bool isRejected = false;

  try {
    wave.Open(make_wav(false, 32, pcm32, 2), wxT("pcm32.wav"));
  } catch (const wxString &) {
    isRejected = true;
  }
  GOAssert(isRejected, "32-bit integer PCM must be rejected");
}

void GOTestWave::TestOwnFormat() {
  const int16_t pcm16[1] = {0};
  const float floatData[1] = {0.1f};
  GOWave wave;

  wave.Open(make_wav(false, 16, pcm16, 1), wxT("pcm16.wav"));
  GOAssert(wave.GetOwnFormat() == GOWave::SF_SIGNEDSHORT_16, "16-bit PCM");
  GOAssert(
    wave.GetMinIntFormat(24) == GOWave::SF_SIGNEDSHORT_16,
    "a shallower file keeps its depth");
  GOAssert(
    wave.GetMinIntFormat(8) == GOWave::SF_SIGNEDBYTE_8,
    "a deeper file is reduced");
  wave.Open(make_wav(true, 32, floatData, 1), wxT("float.wav"));
  GOAssert(wave.GetOwnFormat() == GOWave::SF_IEEE_FLOAT, "float");
  GOAssert(
    wave.GetMinIntFormat(24) == GOWave::SF_SIGNEDINT24_24,
    "a float file is converted to the requested integer depth");
}

void GOTestWave::run() {
  GO_RUN_TEST(TestIntegersUnchanged())
  GO_RUN_TEST(TestFloatBitExact())
  GO_RUN_TEST(TestFloatChannelSelectionAndDownmix())
  GO_RUN_TEST(TestThirtyTwoBitPcmRejected())
  GO_RUN_TEST(TestOwnFormat())
}
