/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTestSignals.h"

#include "sound/buffer/GOSoundBufferPlanarMutable.h"

#include "GOInt.h"

void GOSoundTestSignals::fillWithRamp(std::vector<float> &outData) {
  for (unsigned n = outData.size(), i = 0; i < n; i++)
    outData[i] = (float)i;
}

void GOSoundTestSignals::fillWithRamp(std::vector<GOInt24> &outData) {
  for (unsigned n = outData.size(), i = 0; i < n; i++)
    outData[i] = (int)(i % 0x7fffff);
}

void GOSoundTestSignals::fillWithAlternatingSignal(
  GOSoundBufferPlanarMutable &outBuffer) {
  float *pData = outBuffer.GetData();

  for (unsigned itemI = 0, n = outBuffer.GetNItems(); itemI < n; itemI++)
    pData[itemI] = (itemI % 2 == 0) ? 0.5f : -0.5f;
}
