/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDREVERBPROCESSOR_H
#define GOSOUNDREVERBPROCESSOR_H

#include "sound/processing/GOSoundProcessorTyped.h"
#include "sound/reverb/GOSoundReverb.h"

#include "GOSoundReverbProcessorState.h"

/**
 * Convolution reverb processor for a windchest's processing chain, wrapping
 * the existing sound/reverb/GOSoundReverb convolution engine
 * (zita-convolver) as the framework's first sound/effects/ processor. Built
 * for Stage 5's manual smoke test only - nothing fills a windchest's chain
 * with it in production yet.
 *
 * Splits GOSoundReverb::Setup()'s work along the framework's shared/
 * per-instance line: EnsureSetup() loads and caches the impulse response
 * once (shared, read-only, via GOSoundReverb::loadIRData(), the same helper
 * GOSoundReverb::Setup() itself calls), while CreateTypedState() builds one
 * independent GOSoundReverbProcessorState per chain state from that cached
 * data (cheap, file-I/O-free).
 */
class GOSoundReverbProcessor
  : public GOSoundProcessorTyped<GOSoundReverbProcessorState> {
private:
  friend class GOTestSoundReverbProcessor; // direct access for unit tests

  const GOSoundReverb::ReverbConfig m_config;
  GOSoundReverb::IRData m_irData;
  unsigned m_NChannels = 0;
  unsigned m_NFrames = 0;
  unsigned m_SampleRate = 0;
  /** true once EnsureSetup() has been called at least once, regardless of
   * whether the load succeeded - CreateTypedState()'s precondition. */
  bool m_IsSetupCalled = false;
  /** true only if the most recent load actually succeeded; false forces a
   * retry on the next EnsureSetup() call even with an unchanged format, so
   * a load failure does not get permanently cached as if it were a
   * successful (if silently empty) setup. */
  bool m_IsIRLoaded = false;

public:
  /** @param config The reverb configuration to load the impulse response
   * from; captured by value, read only by EnsureSetup(). */
  explicit GOSoundReverbProcessor(const GOSoundReverb::ReverbConfig &config);

  /**
   * Loads the impulse response via GOSoundReverb::loadIRData() and caches
   * nChannels/nFrames/sampleRate for CreateTypedState(). The impulse
   * response itself depends only on the configured file and sampleRate (it
   * is resampled to sampleRate), so it is only reloaded when sampleRate
   * actually changes (or the previous load failed) - a repeated call with
   * the same sampleRate, even with a different nChannels/nFrames, is a
   * cheap no-op on the file-I/O side, per the GOSoundProcessor::EnsureSetup()
   * contract. A load error is caught and logged the same way
   * GOSoundReverb::Setup() handles it, leaving the chain silently unloaded
   * (an empty impulse response) rather than propagating - and, unlike a
   * successful load, is retried on every subsequent call rather than cached.
   */
  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override;

protected:
  /** @return a new GOSoundReverbProcessorState built from the cached
   * impulse response; asserts EnsureSetup() was already called. */
  std::unique_ptr<GOSoundReverbProcessorState> CreateTypedState()
    const override;

  /** Convolves each channel of buffer in place, mirroring
   * GOSoundReverb::Process()'s per-channel loop. */
  void Process(
    GOSoundReverbProcessorState &state,
    GOSoundBufferPlanarMutable &buffer) const override;
};

#endif /* GOSOUNDREVERBPROCESSOR_H */
