/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundProcessingTestImpls.h"

#include <algorithm>
#include <cassert>

#include "sound/buffer/GOSoundBufferPlanarMutable.h"

GOAddConstProcessor::GOAddConstProcessor(
  float value, std::vector<int> *pLog, int id)
  : m_value(value), p_log(pLog), m_id(id) {}

void GOAddConstProcessor::EnsureSetup(
  unsigned nChannels, unsigned nFrames, unsigned sampleRate) {
  m_NSetups++;
  m_LastNChannels = nChannels;
  m_LastNFrames = nFrames;
  m_LastSampleRate = sampleRate;
}

std::unique_ptr<GOCountingProcessorState> GOAddConstProcessor::
  CreateTypedState() const {
  return std::make_unique<GOCountingProcessorState>();
}

void GOAddConstProcessor::Process(
  GOCountingProcessorState &state, GOSoundBufferPlanarMutable &buffer) const {
  for (unsigned nItems = buffer.GetNItems(), itemI = 0; itemI < nItems; itemI++)
    buffer.GetData()[itemI] += m_value;

  if (p_log)
    p_log->push_back(m_id);
}

GOScaleProcessor::GOScaleProcessor(float value) : m_value(value) {}

std::unique_ptr<GOCountingProcessorState> GOScaleProcessor::CreateTypedState()
  const {
  return std::make_unique<GOCountingProcessorState>();
}

void GOScaleProcessor::Process(
  GOCountingProcessorState &state, GOSoundBufferPlanarMutable &buffer) const {
  for (unsigned nItems = buffer.GetNItems(), itemI = 0; itemI < nItems; itemI++)
    buffer.GetData()[itemI] *= m_value;
}

void GOOnePoleState::Reset() { std::fill(m_prev.begin(), m_prev.end(), 0.0f); }

GOOnePoleProcessor::GOOnePoleProcessor(float alpha) : m_alpha(alpha) {}

std::unique_ptr<GOOnePoleState> GOOnePoleProcessor::CreateTypedState() const {
  assert(m_NChannels && "EnsureSetup() must be called before CreateState()");

  return std::make_unique<GOOnePoleState>(m_NChannels);
}

void GOOnePoleProcessor::Process(
  GOOnePoleState &state, GOSoundBufferPlanarMutable &buffer) const {
  const unsigned nChannels = buffer.GetNChannels();

  assert(nChannels == m_NChannels);

  for (unsigned channelI = 0; channelI < nChannels; channelI++) {
    GOSoundBufferMutableMono channelBuffer = buffer.GetChannelBuffer(channelI);
    float prev = state.GetPrev(channelI);

    for (unsigned nFrames = channelBuffer.GetNFrames(), frameI = 0;
         frameI < nFrames;
         frameI++) {
      const float y
        = m_alpha * channelBuffer.GetData()[frameI] + (1.0f - m_alpha) * prev;

      channelBuffer.GetData()[frameI] = y;
      prev = y;
    }

    state.SetPrev(channelI, prev);
  }
}

GOCountingMapper::GOCountingMapper(
  GOAddConstProcessor &processor, bool isPushingValue, float newValue)
  : GOSoundProcessingPrmMapper(processor),
    m_IsPushingValue(isPushingValue),
    m_NewValue(newValue) {}

void GOCountingMapper::EnsureParametersUpToDate() {
  m_NCalls++;

  if (m_IsPushingValue)
    static_cast<GOAddConstProcessor &>(r_processor).SetValue(m_NewValue);
}
