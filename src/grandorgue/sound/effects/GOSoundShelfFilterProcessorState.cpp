/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundShelfFilterProcessorState.h"

#include <algorithm>

GOSoundShelfFilterProcessorState::GOSoundShelfFilterProcessorState(
  unsigned nChannels)
  : m_LowState(nChannels, 0.0f), m_HighState(nChannels, 0.0f) {}

void GOSoundShelfFilterProcessorState::Reset() {
  std::fill(m_LowState.begin(), m_LowState.end(), 0.0f);
  std::fill(m_HighState.begin(), m_HighState.end(), 0.0f);
  m_WasLowActive = false;
  m_WasHighActive = false;
}
