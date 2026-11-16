/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestWaveFileBuilder.h"

#include <cassert>

#include <wx/file.h>

#include "GOWaveLoop.h"
#include "GOWaveTypes.h"

GOBuffer<uint8_t> GOTestWaveFileBuilder::makeWav(
  bool isFloat,
  unsigned nBits,
  unsigned nChannels,
  const void *pSamples,
  unsigned nFrames,
  const GOWaveLoop *pLoop) {
  assert(nChannels >= 1 && nChannels <= 2);
  assert(isFloat ? nBits == 32 : (nBits == 8 || nBits == 16 || nBits == 24));

  const unsigned nBlockBytes = nChannels * nBits / 8;
  const unsigned nDataBytes = nFrames * nBlockBytes;
  // A chunk with an odd size is followed by a pad byte
  const unsigned nPadBytes = nDataBytes % 2;
  const unsigned nSmplBytes = pLoop ? sizeof(GO_WAVECHUNKHEADER)
      + sizeof(GO_WAVESAMPLERCHUNK) + sizeof(GO_WAVESAMPLERLOOP)
                                    : 0;
  const GO_WAVECHUNKHEADER riff = {
    WAVE_TYPE_RIFF,
    4 + 8 + sizeof(GO_WAVEFORMATPCM) + 8 + nDataBytes + nPadBytes + nSmplBytes};
  const GO_WAVETYPEFIELD wave = WAVE_TYPE_WAVE;
  const GO_WAVECHUNKHEADER fmtHead = {WAVE_TYPE_FMT, sizeof(GO_WAVEFORMATPCM)};
  GO_WAVEFORMATPCM fmt;
  const GO_WAVECHUNKHEADER dataHead = {WAVE_TYPE_DATA, nDataBytes};
  GOBuffer<uint8_t> wav;

  fmt.wf.wFormatTag = isFloat ? 3 : 1;
  fmt.wf.nChannels = nChannels;
  fmt.wf.nSamplesPerSec = 48000;
  fmt.wf.nAvgBytesPerSec = 48000 * nBlockBytes;
  fmt.wf.nBlockAlign = nBlockBytes;
  fmt.wBitsPerSample = nBits;
  wav.Append((const uint8_t *)&riff, sizeof(riff));
  wav.Append((const uint8_t *)&wave, sizeof(wave));
  wav.Append((const uint8_t *)&fmtHead, sizeof(fmtHead));
  wav.Append((const uint8_t *)&fmt, sizeof(fmt));
  wav.Append((const uint8_t *)&dataHead, sizeof(dataHead));
  wav.Append((const uint8_t *)pSamples, nDataBytes);
  if (nPadBytes) {
    const uint8_t pad = 0;

    wav.Append(&pad, 1);
  }
  if (pLoop) {
    const GO_WAVECHUNKHEADER smplHead = {
      WAVE_TYPE_SAMPLE,
      sizeof(GO_WAVESAMPLERCHUNK) + sizeof(GO_WAVESAMPLERLOOP)};
    GO_WAVESAMPLERCHUNK smpl = {};
    GO_WAVESAMPLERLOOP loop = {};

    smpl.dwSamplePeriod = 1000000000 / 48000;
    smpl.dwMIDIUnityNote = 60;
    smpl.cSampleLoops = 1;
    loop.dwStart = pLoop->m_StartPosition;
    loop.dwEnd = pLoop->m_EndPosition;
    wav.Append((const uint8_t *)&smplHead, sizeof(smplHead));
    wav.Append((const uint8_t *)&smpl, sizeof(smpl));
    wav.Append((const uint8_t *)&loop, sizeof(loop));
  }
  return wav;
}

void GOTestWaveFileBuilder::writeWavFile(
  const wxString &path,
  bool isFloat,
  unsigned nBits,
  unsigned nChannels,
  const void *pSamples,
  unsigned nFrames,
  const GOWaveLoop *pLoop) {
  const GOBuffer<uint8_t> wav
    = makeWav(isFloat, nBits, nChannels, pSamples, nFrames, pLoop);
  wxFile file(path, wxFile::write);

  assert(file.IsOpened());
  file.Write(wav.get(), wav.GetSize());
}
