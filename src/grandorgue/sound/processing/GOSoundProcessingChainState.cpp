/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundProcessingChainState.h"

#include <cassert>

#include "GOSoundProcessor.h"
#include "GOSoundProcessorState.h"

GOSoundProcessingChainState::GOSoundProcessingChainState(
  const GOSoundProcessingChain &chain)
  : r_chain(chain), m_ChainSignatureAtCreation(chain.GetSignature()) {
  for (unsigned nProcessors = chain.GetNProcessors(), processorI = 0;
       processorI < nProcessors;
       processorI++)
    m_states.push_back(chain.GetProcessor(processorI).CreateState());
}

GOSoundProcessingChainState::~GOSoundProcessingChainState() = default;

void GOSoundProcessingChainState::Reset() {
  for (const std::unique_ptr<GOSoundProcessorState> &pState : m_states)
    pState->Reset();
}

void GOSoundProcessingChainState::Process(GOSoundBufferPlanarMutable &buffer) {
  assert(IsValid());

  for (unsigned nStates = (unsigned)m_states.size(), stateI = 0;
       stateI < nStates;
       stateI++)
    r_chain.GetProcessor(stateI).Process(*m_states[stateI], buffer);
}
