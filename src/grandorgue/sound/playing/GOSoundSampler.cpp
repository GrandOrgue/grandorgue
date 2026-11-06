/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundSampler.h"

#include <cassert>

#include "sound/dsp-kernels/GOSoundResample.h"

void GOSoundSampler::InitVolumeFactor(float volumeFactor) {
  m_InitialVolumeFactor = volumeFactor;
  m_AppliedVolumeFactor = volumeFactor;
  m_AppliedVolumeRatio = 1.0f;
}

void GOSoundSampler::EnsureVolumeFactor(float volumeFactor) {
  if (volumeFactor != m_AppliedVolumeFactor) {
    // A sampler that was started silent can not be raised later: its fader
    // target volume is 0
    m_AppliedVolumeRatio = m_InitialVolumeFactor > 0.0f
      ? volumeFactor / m_InitialVolumeFactor
      : 1.0f;
    m_AppliedVolumeFactor = volumeFactor;
  }
}

void GOSoundSampler::InitResamplerIncrement(
  float tuningFactor, unsigned resamplerIncrement) {
  assert(tuningFactor > 0.0f);
  m_InitialTuningFactor = tuningFactor;
  m_InitialResamplerIncrement = resamplerIncrement;
  m_AppliedTuningFactor = tuningFactor;
  m_AppliedResamplerIncrement = resamplerIncrement;
}

void GOSoundSampler::EnsureTuningFactor(float tuningFactor) {
  assert(tuningFactor > 0.0f);
  assert(m_InitialTuningFactor > 0.0f);
  if (tuningFactor != m_AppliedTuningFactor) {
    m_AppliedResamplerIncrement = GOSoundResample::scaleResamplerPosIncrement(
      m_InitialResamplerIncrement, tuningFactor / m_InitialTuningFactor);
    m_AppliedTuningFactor = tuningFactor;
  }
}
