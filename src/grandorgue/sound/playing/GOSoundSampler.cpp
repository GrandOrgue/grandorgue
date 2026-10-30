/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundSampler.h"

#include <cassert>

#include "sound/dsp-kernels/GOSoundResample.h"

void GOSoundSampler::InitResamplerIncrement(
  float tuning, unsigned resamplerIncrement) {
  assert(tuning > 0.0f);
  m_TargetTuning = tuning;
  m_TargetResamplerIncrement = resamplerIncrement;
  m_AppliedResamplerIncrement = resamplerIncrement;
}

unsigned GOSoundSampler::GetResamplerIncrementFor(float tuning) const {
  return tuning == m_TargetTuning
    ? m_TargetResamplerIncrement
    : GOSoundResample::scaleFractionIncrement(
      m_TargetResamplerIncrement, tuning / m_TargetTuning);
}
