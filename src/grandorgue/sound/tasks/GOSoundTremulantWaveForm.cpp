/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantWaveForm.h"

#include <algorithm>
#include <cassert>
#include <cmath>

GOSoundTremulantWaveForm::GOSoundTremulantWaveForm(
  const GOTremulant::Description &desc, unsigned nChunkFrames)
  : m_NLoopFrames(desc.nPeriodFrames),
    m_StartRamp(true, desc.nStartFrames),
    m_StopRamp(false, desc.nStopFrames) {
  assert(m_NLoopFrames > 0);

  // An empty ampDeviation leaves the loop not built
  if (!desc.ampDeviation.empty()) {
    assert(desc.ampDeviation.size() == m_NLoopFrames);
    m_AmpUnits.Resize(m_NLoopFrames, nChunkFrames);

    uint16_t *pAmpUnits = m_AmpUnits.GetLoopData();
    const float *pDeviation = desc.ampDeviation.data();

    for (unsigned frameI = 0; frameI < m_NLoopFrames;
         ++frameI, ++pAmpUnits, ++pDeviation)
      *pAmpUnits = (uint16_t)std::clamp(
        std::lround((1.0f + *pDeviation) * UNIT_AMP_F), 0L, (long)UINT16_MAX);
    m_AmpUnits.FillTail();
  }

  // An empty pitchDeviation leaves the curve not built
  if (!desc.pitchDeviation.empty()) {
    assert(desc.pitchDeviation.size() == m_NLoopFrames);
    m_PitchRateCurve.Build(
      desc.pitchDeviation.data(), m_NLoopFrames, nChunkFrames);
  }
}
