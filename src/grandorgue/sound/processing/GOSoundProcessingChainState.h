/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSINGCHAINSTATE_H
#define GOSOUNDPROCESSINGCHAINSTATE_H

#include <memory>
#include <vector>

#include "GOSoundProcessingChain.h"

class GOSoundBufferPlanarMutable;
class GOSoundProcessorState;

/**
 * One independent DSP-memory instance of a GOSoundProcessingChain: exactly
 * one GOSoundProcessorState per processor of the chain, in the same order.
 * A chain owner (e.g. a windchest group) creates one of these per audio
 * group, all sharing the same GOSoundProcessingChain: this is what lets one
 * GOSoundProcessor instance serve every audio group of its windchest, while
 * each audio group's DSP memory stays independent. The chain must outlive
 * every GOSoundProcessingChainState created from it.
 */
class GOSoundProcessingChainState {
private:
  friend class GOTestSoundProcessingChain; // direct access for unit tests
  friend class GOTestSoundProcessorTyped;  // direct access for unit tests

  // The chain this state was created from; must outlive this state. Reused
  // by Process() to reach GetProcessor(stateI) for each state stored below.
  const GOSoundProcessingChain &r_chain;
  // r_chain's signature at construction time; compared against its current
  // signature by IsValid() to detect that r_chain has since been mutated
  // (topology and/or format) out from under this state.
  const size_t m_ChainSignatureAtCreation;
  // One state per processor of r_chain, same order; each touched only by
  // this chain state's own Process() calls, never shared with another
  // chain state.
  std::vector<std::unique_ptr<GOSoundProcessorState>> m_states;

public:
  /** Builds one state per processor of the chain, via each processor's
   * CreateState(). */
  explicit GOSoundProcessingChainState(const GOSoundProcessingChain &chain);

  // Defined in the .cpp: a vector<unique_ptr<GOSoundProcessorState>>'s
  // implicit destructor needs GOSoundProcessorState's complete definition,
  // which this header intentionally doesn't include (only forward-declares
  // it).
  ~GOSoundProcessingChainState();

  GOSoundProcessingChainState(const GOSoundProcessingChainState &) = delete;
  GOSoundProcessingChainState &operator=(const GOSoundProcessingChainState &)
    = delete;

  unsigned GetNStates() const { return (unsigned)m_states.size(); }

  /** @return whether r_chain's current signature (topology and format)
   * still matches what this state was created for. Process() asserts this;
   * see GOSoundProcessingChain's class doc for the lifecycle this
   * protects against. */
  bool IsValid() const {
    return m_ChainSignatureAtCreation == r_chain.GetSignature();
  }

  /** Resets the DSP memory of every processor state. */
  void Reset();

  /**
   * Runs every processor of the chain, in order, in place on the buffer.
   * An empty chain leaves the buffer untouched (the loop is empty). Not
   * const: this state is owned by exactly one caller (never shared), and
   * it mutates the DSP memory of every GOSoundProcessorState it holds.
   */
  void Process(GOSoundBufferPlanarMutable &buffer);
};

#endif /* GOSOUNDPROCESSINGCHAINSTATE_H */
