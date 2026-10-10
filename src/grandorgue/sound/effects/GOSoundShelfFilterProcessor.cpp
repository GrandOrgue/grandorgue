/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundShelfFilterProcessor.h"

#include <algorithm>
#include <cassert>
#include <vector>

#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

void GOSoundShelfFilterProcessor::EnsureSetup(
  unsigned nChannels, unsigned nFrames, unsigned sampleRate) {
  if (!m_IsSetupCalled || sampleRate != m_SampleRate) {
    m_SampleRate = sampleRate;
    GOSoundOnePoleFilter::computeCoeffs(
      GOSoundOnePoleFilter::Type::TYPE_LOW_SHELF,
      m_LowFrequency,
      m_LowGain,
      m_SampleRate,
      m_LowCoeffs);
    GOSoundOnePoleFilter::computeCoeffs(
      GOSoundOnePoleFilter::Type::TYPE_HIGH_SHELF,
      m_HighFrequency,
      m_HighGain,
      m_SampleRate,
      m_HighCoeffs);
  }
  m_NChannels = nChannels;
  m_IsSetupCalled = true;
}

void GOSoundShelfFilterProcessor::SetLowShelf(double frequency, double gain) {
  m_LowFrequency = frequency;
  m_LowGain = gain;
  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_LOW_SHELF,
    m_LowFrequency,
    m_LowGain,
    m_SampleRate,
    m_LowCoeffs);
}

void GOSoundShelfFilterProcessor::SetHighShelf(double frequency, double gain) {
  m_HighFrequency = frequency;
  m_HighGain = gain;
  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_HIGH_SHELF,
    m_HighFrequency,
    m_HighGain,
    m_SampleRate,
    m_HighCoeffs);
}

std::unique_ptr<GOSoundShelfFilterProcessorState> GOSoundShelfFilterProcessor::
  CreateTypedState() const {
  assert(
    m_IsSetupCalled && "EnsureSetup() must be called before CreateState()");

  return std::make_unique<GOSoundShelfFilterProcessorState>(m_NChannels);
}

/**
 * Records whether a band is active in this round. When it has just become
 * inactive, clears its state: an identity filter would leave the state at 0,
 * and a stale state would otherwise leak into the audio (a click) when the
 * band is activated again.
 */
static void update_band_memory(
  bool isActive, bool &ioWasActive, std::vector<float> &ioBandState) {
  if (!isActive && ioWasActive)
    std::fill(ioBandState.begin(), ioBandState.end(), 0.0f);
  ioWasActive = isActive;
}

void GOSoundShelfFilterProcessor::Process(
  GOSoundShelfFilterProcessorState &state,
  GOSoundBufferPlanarMutable &buffer) const {
  update_band_memory(
    m_LowCoeffs.isActive, state.m_WasLowActive, state.m_LowState);
  update_band_memory(
    m_HighCoeffs.isActive, state.m_WasHighActive, state.m_HighState);
  if (m_LowCoeffs.isActive || m_HighCoeffs.isActive) {
    const unsigned nChannels = buffer.GetNChannels();

    assert(nChannels == state.m_LowState.size());
    assert(nChannels == state.m_HighState.size());

    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      GOSoundBufferMutableMono channelBuffer
        = buffer.GetChannelBuffer(channelI);
      float *pData = channelBuffer.GetData();

      for (unsigned n = channelBuffer.GetNFrames(); n > 0; n--, pData++) {
        if (m_LowCoeffs.isActive)
          *pData = GOSoundOnePoleFilter::processSample(
            m_LowCoeffs, *pData, state.m_LowState[channelI]);
        if (m_HighCoeffs.isActive)
          *pData = GOSoundOnePoleFilter::processSample(
            m_HighCoeffs, *pData, state.m_HighState[channelI]);
      }
    }
  }
}
