/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTWAVEFILEBUILDER_H
#define GOTESTWAVEFILEBUILDER_H

#include <cstdint>

#include <wx/string.h>

#include "GOBuffer.h"

struct GOWaveLoop;

/** Builds RIFF/WAVE files for tests of the wave loading. */
class GOTestWaveFileBuilder {
public:
  /**
   * @param isFloat whether the items are IEEE floats (format code 3) or PCM (1)
   * @param nBits the bits per item: 8, 16, 24 or 32
   * @param nChannels the number of channels, 1 or 2
   * @param pSamples the interleaved items, already in the file layout
   * @param nFrames the number of frames
   * @param pLoop the loop to store in a smpl chunk; if null, no smpl chunk
   * @return a RIFF/WAVE file at 48 kHz
   */
  static GOBuffer<uint8_t> makeWav(
    bool isFloat,
    unsigned nBits,
    unsigned nChannels,
    const void *pSamples,
    unsigned nFrames,
    const GOWaveLoop *pLoop = nullptr);

  /**
   * Writes the result of makeWav() to a file.
   * @param path the new file
   */
  static void writeWavFile(
    const wxString &path,
    bool isFloat,
    unsigned nBits,
    unsigned nChannels,
    const void *pSamples,
    unsigned nFrames,
    const GOWaveLoop *pLoop = nullptr);
};

#endif /* GOTESTWAVEFILEBUILDER_H */
