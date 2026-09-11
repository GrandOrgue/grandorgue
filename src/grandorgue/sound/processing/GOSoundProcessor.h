/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSOR_H
#define GOSOUNDPROCESSOR_H

#include <memory>

class GOSoundBufferPlanarMutable;
class GOSoundProcessorState;

/**
 * A DSP unit whose own instance holds only parameters (written by a
 * GOSoundProcessingPrmMapper between rounds, read but never written by
 * Process()); all mutable DSP memory lives in the GOSoundProcessorState
 * objects it creates via CreateState(). This split is what lets a single
 * GOSoundProcessor instance be shared, read-only, by every
 * GOSoundProcessingChainState of the chain it belongs to.
 *
 * Call order contract: CreateState() has no format parameters of its own,
 * so a processor whose state depends on the audio format (e.g. one entry
 * per channel) must have EnsureSetup() called at least once, with the
 * format it will be used with, before CreateState() — and should assert
 * that in both CreateState() and Process(). A processor indifferent to the
 * format is exempt. Either way, CreateState() must allocate everything the
 * state will ever need; Process() must not allocate (it runs on the audio
 * thread).
 */
class GOSoundProcessor {
public:
  virtual ~GOSoundProcessor() = default;

  /**
   * Idempotent configuration of the audio format. Repeated calls with the
   * same arguments must be cheap no-ops; a call with different arguments
   * must reconfigure. Called from the setup thread, never from Process().
   * @param nChannels Number of audio channels the buffers passed to
   *   Process() will have
   * @param nFrames Number of frames per channel the buffers passed to
   *   Process() will have
   * @param sampleRate Audio sample rate, in Hz
   */
  virtual void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate)
    = 0;

  /** @return a fresh, already-reset DSP-memory object for one chain state */
  virtual std::unique_ptr<GOSoundProcessorState> CreateState() const = 0;

  /**
   * Processes the buffer in place, using and updating only the given state.
   * const: this instance may be shared by many GOSoundProcessingChainState
   * objects at once, so it may only read its own parameters here, never
   * write them.
   * @param state The DSP memory of the calling chain state; must have been
   *   created by this processor's CreateState()
   * @param buffer The buffer to process in place
   */
  virtual void Process(
    GOSoundProcessorState &state, GOSoundBufferPlanarMutable &buffer) const = 0;
};

#endif /* GOSOUNDPROCESSOR_H */
