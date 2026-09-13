/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundProcessingChain.h"

#include <algorithm>
#include <cassert>

#include "GOSoundProcessingChainState.h"
#include "GOSoundProcessingPrmMapper.h"
#include "GOSoundProcessor.h"

// Defined here, not inline in the header: even the default constructor
// needs GOSoundProcessor's and GOSoundProcessingPrmMapper's complete
// definitions, to generate the member-destruction code for the case where
// a later member's construction throws; the header intentionally only
// forward-declares them.
GOSoundProcessingChain::GOSoundProcessingChain() = default;
GOSoundProcessingChain::~GOSoundProcessingChain() = default;

const GOSoundProcessor &GOSoundProcessingChain::GetProcessor(
  unsigned processorI) const {
  assert(processorI < m_processors.size());
  return *m_processors[processorI];
}

void GOSoundProcessingChain::AddProcessor(
  std::unique_ptr<GOSoundProcessor> pProcessor) {
  InsertProcessor(GetNProcessors(), std::move(pProcessor));
}

void GOSoundProcessingChain::InsertProcessor(
  unsigned position, std::unique_ptr<GOSoundProcessor> pProcessor) {
  assert(!m_IsSetUp);
  assert(position <= m_processors.size());

  m_processors.insert(m_processors.begin() + position, std::move(pProcessor));
  m_Generation++;
}

std::unique_ptr<GOSoundProcessor> GOSoundProcessingChain::RemoveProcessor(
  unsigned position) {
  assert(!m_IsSetUp);
  assert(position < m_processors.size());

  const GOSoundProcessor &processor = *m_processors[position];

  for (const std::unique_ptr<GOSoundProcessingPrmMapper> &pMapper : m_mappers)
    assert(
      &pMapper->GetProcessor() != &processor
      && "remove the mapper(s) referencing this processor first, via "
         "RemoveMapper()");

  std::unique_ptr<GOSoundProcessor> pRemoved
    = std::move(m_processors[position]);

  m_processors.erase(m_processors.begin() + position);
  m_Generation++;

  return pRemoved;
}

void GOSoundProcessingChain::AddMapper(
  std::unique_ptr<GOSoundProcessingPrmMapper> pMapper) {
  assert(!m_IsSetUp);
  assert(
    std::any_of(
      m_processors.begin(),
      m_processors.end(),
      [&pMapper](const std::unique_ptr<GOSoundProcessor> &pProcessor) {
        return pProcessor.get() == &pMapper->GetProcessor();
      })
    && "the mapper's target processor must already be in this chain");

  m_mappers.push_back(std::move(pMapper));
}

std::unique_ptr<GOSoundProcessingPrmMapper> GOSoundProcessingChain::
  RemoveMapper(const GOSoundProcessingPrmMapper &mapper) {
  assert(!m_IsSetUp);

  auto it = std::find_if(
    m_mappers.begin(),
    m_mappers.end(),
    [&mapper](const std::unique_ptr<GOSoundProcessingPrmMapper> &pMapper) {
      return pMapper.get() == &mapper;
    });

  assert(it != m_mappers.end() && "mapper not found in this chain");

  std::unique_ptr<GOSoundProcessingPrmMapper> pRemoved = std::move(*it);

  m_mappers.erase(it);

  return pRemoved;
}

void GOSoundProcessingChain::ClearChain() {
  assert(!m_IsSetUp);

  // Mappers first: a mapper may hold a raw (non-owning) pointer to one of
  // the chain's processors, so the referrer is cleared before the
  // referent.
  m_mappers.clear();
  m_processors.clear();
  m_Generation++;
}

void GOSoundProcessingChain::EnsureSetup(
  unsigned nChannels, unsigned nFrames, unsigned sampleRate) {
  const bool isFormatChanged = !m_IsSetUp || nChannels != m_NChannels
    || nFrames != m_NFrames || sampleRate != m_SampleRate;

  if (isFormatChanged) {
    // Bumped before touching any processor, not after the loop: if a
    // later processor's EnsureSetup() throws, any state still alive from
    // before this call must already read as invalid, since some
    // processors may have already been reconfigured to the new format.
    m_Generation++;
    // Cleared before the loop, not just left alone: if a later processor's
    // EnsureSetup() throws, this chain must not look fully set up again
    // until every processor has actually completed with the new format -
    // CreateState() must not be allowed to build a state against processors
    // left configured for a mix of the old and new format. It also forces
    // isFormatChanged to be recomputed as true on any later retry
    // (including a retry with the old, previously-stored format), since
    // m_NChannels/m_NFrames/m_SampleRate below were never actually applied
    // to every processor.
    m_IsSetUp = false;

    for (const std::unique_ptr<GOSoundProcessor> &pProcessor : m_processors)
      pProcessor->EnsureSetup(nChannels, nFrames, sampleRate);

    m_NChannels = nChannels;
    m_NFrames = nFrames;
    m_SampleRate = sampleRate;
    m_IsSetUp = true;
  }
}

void GOSoundProcessingChain::Cleanup() {
  m_NChannels = 0;
  m_NFrames = 0;
  m_SampleRate = 0;
  m_IsSetUp = false;
  m_Generation++;
}

void GOSoundProcessingChain::EnsureParametersUpToDate() {
  assert(m_IsSetUp);

  for (const std::unique_ptr<GOSoundProcessingPrmMapper> &pMapper : m_mappers)
    pMapper->EnsureParametersUpToDate();
}

std::unique_ptr<GOSoundProcessingChainState> GOSoundProcessingChain::
  CreateState() const {
  assert(m_IsSetUp && "EnsureSetup() must be called before CreateState()");

  return std::make_unique<GOSoundProcessingChainState>(*this);
}
