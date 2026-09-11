/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSINGCHAIN_H
#define GOSOUNDPROCESSINGCHAIN_H

#include <cstddef>
#include <memory>
#include <vector>

class GOSoundProcessingChainState;
class GOSoundProcessingPrmMapper;
class GOSoundProcessor;

/**
 * An ordered list of GOSoundProcessor%s, run in that order by every
 * GOSoundProcessingChainState created from this chain, plus the
 * GOSoundProcessingPrmMapper%s that keep those processors' parameters up to
 * date. Owns both lists as two independent vectors rather than a paired
 * struct: cardinality between processors and mappers is not 1:1 (a
 * processor may need no mapper) and mapper order carries no meaning, unlike
 * processor order, which is the DSP signal path.
 *
 * Lifecycle (enforced by assert(), not by the type system):
 *  1. Build phase: add processors/mappers via AddProcessor(),
 *     InsertProcessor(), AddMapper(); remove via RemoveProcessor() or
 *     RemoveMapper(). Allowed only while !m_IsSetUp (asserted).
 *     RemoveProcessor() additionally asserts that no remaining mapper still
 *     references the processor being removed (RemoveMapper() it first).
 *  2. EnsureSetup(): forwards the audio format to every processor
 *     currently in the chain. May be called repeatedly, with the same or
 *     different arguments each time (every processor must treat a
 *     same-arguments call as a cheap no-op, per GOSoundProcessor's own
 *     contract). Its first call sets m_IsSetUp, which from then on forbids
 *     any further topology mutation (step 1) until step 4.
 *  3. CreateState() / EnsureParametersUpToDate(): CreateState() builds a
 *     GOSoundProcessingChainState; asserts m_IsSetUp. May be called any
 *     number of times - once per independent DSP-memory instance (e.g. one
 *     per audio group of a windchest) - all sharing this one chain.
 *     EnsureParametersUpToDate() forwards to every mapper, pushing fresh
 *     parameter values into the processors ahead of a Process() round;
 *     also asserts m_IsSetUp, and is expected to be called repeatedly,
 *     once per round, throughout this step.
 *  4. Cleanup() (optional): the exact inverse of step 2 - clears the
 *     stored format and resets m_IsSetUp to false, re-entering the build
 *     phase (step 1). Does not touch processors/mappers.
 *  5. ClearChain() (optional, only meaningful after step 4, since it too
 *     requires !m_IsSetUp): the exact inverse of step 1 - removes every
 *     processor and mapper, returning the chain to an empty topology, as
 *     if freshly constructed.
 *
 * Any GOSoundProcessingChainState still alive from step 3 becomes invalid
 * (IsValid() false) the moment step 4 or 5 runs, since what it was built
 * for (the format, the topology) is no longer intact. From step 4 or 5,
 * return to step 1 to build the chain anew. Except for the build/set-up
 * split above, the chain does not track which GOSoundProcessingChainState
 * objects were created from it, or whether any are still alive: mutating
 * the chain while a state is alive is not prevented, only detected
 * reactively by that state's own IsValid().
 */
class GOSoundProcessingChain {
private:
  friend class GOTestSoundProcessingChain; // direct access for unit tests

  // Owned processors, in signal-path order; Process() walks them in this
  // order.
  std::vector<std::unique_ptr<GOSoundProcessor>> m_processors;
  // Owned mappers; order carries no meaning (each touches disjoint
  // parameters of the processors above).
  std::vector<std::unique_ptr<GOSoundProcessingPrmMapper>> m_mappers;
  // Format from the most recent EnsureSetup() call, or all-zero before the
  // first call since construction or the last Cleanup(); compared against
  // on each EnsureSetup() call, purely to decide whether m_Generation needs
  // bumping (see below) - not otherwise read.
  unsigned m_NChannels = 0;
  unsigned m_NFrames = 0;
  unsigned m_SampleRate = 0;
  // Monotonically increasing counter, bumped by every operation that
  // actually changes something a live GOSoundProcessingChainState depends
  // on: InsertProcessor(), RemoveProcessor(), ClearChain(), Cleanup(), and
  // an EnsureSetup() call whose arguments differ from the previous one (or
  // is the first call since construction/Cleanup()). Returned as-is by
  // GetSignature(), instead of hashing processor addresses: once a removed
  // processor is destroyed, the allocator can reuse its address for a
  // later one, which would let a stale state's snapshot coincidentally
  // match a live but different chain. A counter that only ever increases
  // cannot repeat a value already seen from this chain, so it cannot
  // collide this way.
  size_t m_Generation = 0;
  // True once EnsureSetup() has been called at least once since
  // construction or the last Cleanup(); guards the build-phase/set-up-phase
  // split documented above.
  bool m_IsSetUp = false;

public:
  GOSoundProcessingChain();
  ~GOSoundProcessingChain();

  GOSoundProcessingChain(const GOSoundProcessingChain &) = delete;
  GOSoundProcessingChain &operator=(const GOSoundProcessingChain &) = delete;

  unsigned GetNProcessors() const { return (unsigned)m_processors.size(); }

  /** @return whether this chain has no processors (an empty state's Process()
   * is then a no-op) */
  bool IsEmpty() const { return m_processors.empty(); }

  /**
   * @param processorI Position in the signal path (0-based)
   * @return the processor at that position
   */
  const GOSoundProcessor &GetProcessor(unsigned processorI) const;

  /**
   * @return an opaque value for GOSoundProcessingChainState's own staleness
   *   self-check only: currently just m_Generation, but callers must treat
   *   it as opaque, not as a generation number, since that's an
   *   implementation detail. Not a stable or serializable value.
   */
  size_t GetSignature() const { return m_Generation; }

  /** Step 1. Appends a processor; it will run after all previously added
   * ones. Equivalent to InsertProcessor(GetNProcessors(), ...). Only
   * allowed while !m_IsSetUp (asserted); see the class doc. */
  void AddProcessor(std::unique_ptr<GOSoundProcessor> pProcessor);

  /** Step 1. Inserts a processor at the given 0-based position in the
   * signal path; every processor previously at or after that position
   * shifts one step later. Only allowed while !m_IsSetUp (asserted); see
   * the class doc. */
  void InsertProcessor(
    unsigned position, std::unique_ptr<GOSoundProcessor> pProcessor);

  /** Step 1. Removes the processor at the given position and returns
   * ownership to the caller; every later processor shifts one step
   * earlier. Only allowed while !m_IsSetUp (asserted); see the class doc.
   * Also asserts that no remaining mapper still references this processor
   * (RemoveMapper() it first), since returning ownership to the caller may
   * lead to the processor being destroyed, which would leave such a
   * mapper's reference dangling. */
  std::unique_ptr<GOSoundProcessor> RemoveProcessor(unsigned position);

  /** Step 1. Appends a mapper; mapper order is irrelevant (they touch
   * disjoint parameters). Only allowed while !m_IsSetUp (asserted); see
   * the class doc. Also asserts that the mapper's GetProcessor() is
   * already one of this chain's own processors, so a mapper can never
   * outlive or reach outside the chain it's installed on. */
  void AddMapper(std::unique_ptr<GOSoundProcessingPrmMapper> pMapper);

  /** Step 1. Removes the given mapper (by identity) and returns ownership
   * to the caller. Only allowed while !m_IsSetUp (asserted); see the class
   * doc. */
  std::unique_ptr<GOSoundProcessingPrmMapper> RemoveMapper(
    const GOSoundProcessingPrmMapper &mapper);

  /** Step 5. The exact inverse of step 1: removes every processor and
   * mapper, returning the chain to an empty topology. Only allowed while
   * !m_IsSetUp (asserted); see the class doc. */
  void ClearChain();

  /** Step 2. Forwards EnsureSetup() to every processor, in order; see the
   * class doc for the full lifecycle. */
  void EnsureSetup(unsigned nChannels, unsigned nFrames, unsigned sampleRate);

  /** Step 4. The exact inverse of step 2; see the class doc. */
  void Cleanup();

  /** Step 3. Forwards EnsureParametersUpToDate() to every mapper. Asserts
   * m_IsSetUp; see the class doc. */
  void EnsureParametersUpToDate();

  /** Step 3. @return a new DSP-memory instance of this chain: one state
   * per processor. Asserts EnsureSetup() has been called at least once;
   * see the class doc. */
  std::unique_ptr<GOSoundProcessingChainState> CreateState() const;
};

#endif /* GOSOUNDPROCESSINGCHAIN_H */
