/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDREVERBPROCESSORSTATE_H
#define GOSOUNDREVERBPROCESSORSTATE_H

#include <memory>
#include <vector>

#include "sound/processing/GOSoundProcessorState.h"
#include "sound/reverb/GOSoundReverb.h"

class Convproc;
class GOSoundReverbProcessor;

/**
 * Per-chain-state DSP memory of GOSoundReverbProcessor: one Convproc
 * convolution engine per channel, configured and fed from the processor's
 * cached impulse response (GOSoundReverbProcessor::EnsureSetup()). Built
 * entirely from data already loaded by EnsureSetup() - no file I/O, matching
 * CreateState()'s off-audio-thread, allocation-only contract.
 */
class GOSoundReverbProcessorState : public GOSoundProcessorState {
private:
  friend class GOSoundReverbProcessor;     // Process() needs direct access to
                                           // mp_ConvprocsByChannel
  friend class GOTestSoundReverbProcessor; // direct access for unit tests

  /** One owned Convproc convolution engine per channel, indexed by channel -
   * the per-chain-state twin of GOSoundReverb::m_engine. Empty (rather than
   * one entry per channel) when irData had nothing valid to convolve, or a
   * channel's configure() call failed - see the constructor. Process()/
   * Reset() both treat an empty vector as a no-op state. */
  std::vector<std::unique_ptr<Convproc>> mp_ConvprocsByChannel;

public:
  /**
   * Builds one Convproc per channel, configures it for nFrames, and feeds it
   * irData via impdata_create() - the per-channel half of what
   * GOSoundReverb::Setup() used to do in one call, now replayable with no
   * file I/O since irData is already loaded. Leaves mp_ConvprocsByChannel
   * empty instead - a safe no-op state - if irData.data is empty (nothing to
   * convolve) or any channel's configure() call fails, the same way
   * GOSoundReverb::Setup() clears m_engine when it cannot load its impulse
   * response.
   * @param irData The impulse response to feed every engine
   * @param nChannels Number of Convproc engines to build (one per channel)
   * @param nFrames Frames per buffer Process() will be called with
   */
  GOSoundReverbProcessorState(
    const GOSoundReverb::IRData &irData, unsigned nChannels, unsigned nFrames);
  // Defined in the .cpp file because std::vector<std::unique_ptr<Convproc>>'s
  // destructor requires the complete type of Convproc, which this header
  // intentionally doesn't include (only forward-declared).
  ~GOSoundReverbProcessorState() override;

  /** Resets every channel's Convproc, discarding its accumulated tail. */
  void Reset() override;
};

#endif /* GOSOUNDREVERBPROCESSORSTATE_H */
