/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDVIBRATOPROCESSOR_H
#define GOSOUNDVIBRATOPROCESSOR_H

#include <cstdint>
#include <memory>

#include "sound/buffer/GOSoundBufferPlanar.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"
#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/processing/GOSoundProcessorTyped.h"

#include "GOSoundVibratoPitchRateCurve.h"
#include "GOSoundVibratoProcessorState.h"

class GOTestPerfSoundVibratoProcessor;
class GOTestSoundVibratoProcessor;

/**
 * Effect processor for a windchest's processing chain: reads the audio back
 * from a delay buffer with the per-frame resampling rates of a chunk of a
 * GOSoundVibratoPitchRateCurve, which makes a Doppler-shift pitch vibrato -
 * the pitch half of a synthesized tremulant (issue #709). Built on
 * GOSoundVibratoProcessorState's delay buffer and
 * GOSoundResample::RingPlanarFrameVector /
 * ResampleBlockVariableRatePlanar().
 *
 * Holds only parameters (the format, the interpolation type, the ramps) and the
 * chunk of the rates for the current audio period; every mutable byte a chain
 * needs lives in GOSoundVibratoProcessorState, since Process() is const and one
 * processor instance is shared across many chain states. The chunk is the same
 * for all the states: it is chosen once per audio period and set with
 * SetPitchRateChunk(). A chunk with no rates
 * bypasses at zero cost - an organ with no pitch tremulant pays nothing for
 * this processor being in the chain.
 *
 * The curve is normalised, so the read position does not drift against the
 * write position. The delay is not fixed: it starts at its minimum
 * (N_LOOKAHEAD_FRAMES) and settles at what the curve needs; the portions of the
 * first cycle that would shrink the delay below the minimum are rescaled
 * (GetResamplerPositionIncrementsForUse()).
 *
 * Terms: the "undelayed" signal is the input of a period as it arrives in the
 * buffer; the "delayed" signal is the one resampled from the delay buffer. When
 * the rates appear the output goes from the undelayed signal to the delayed
 * one (the activation crossfade), and when they disappear back (the
 * deactivation crossfade) - a plain switch would click, since the delayed
 * signal lags by up
 * to the maximum lag. During a crossfade the lag is frozen: the read
 * position advances at the rate of the write one. The ramps of the crossfades
 * (m_ActivationRamp, m_DeactivationRamp) are the weights of the undelayed
 * signal; the delayed signal gets the rest. The phase of the state
 * (GOSoundVibratoProcessorState::ActivatingPhase) tells which signal is heard.
 */
class GOSoundVibratoProcessor
  : public GOSoundProcessorTyped<GOSoundVibratoProcessorState> {
public:
  /**
   * The default maximum extra lag the read position may take on top of its
   * minimum, in milliseconds. It sizes the delay ring, so it is a parameter of
   * the constructor.
   */
  static constexpr unsigned DEFAULT_MAX_LAG_MS = 100;

  /**
   * The default length of the activation crossfade (the undelayed signal
   * replaced by the delayed one when the tremulant is switched on), in
   * milliseconds. Short: the pitch modulation should start soon, and the
   * delayed signal enters with the minimum lag, so nothing comb-filters.
   */
  static constexpr unsigned DEFAULT_ACTIVATION_CROSSFADE_MS = 3;

  /**
   * The default length of the deactivation crossfade (the delayed signal
   * replaced by the undelayed one when the tremulant is switched off), in
   * milliseconds. During it the lag is frozen at the value it had at the
   * switch-off, so the sum of the delayed and the undelayed signals is a comb
   * filter: the shorter the crossfade, the shorter the audible flanging. It is
   * still much longer than any period of the audible band, so there is no
   * click.
   */
  static constexpr unsigned DEFAULT_DEACTIVATION_CROSSFADE_MS = 10;

  /**
   * The maximum length of a crossfade, in milliseconds: a longer one makes no
   * sense. Asserted by the setters of the lengths.
   */
  static constexpr unsigned MAX_CROSSFADE_MS = 10000;

private:
  friend class GOTestSoundVibratoProcessor;     // direct access for unit tests
  friend class GOTestPerfSoundVibratoProcessor; // the same for the perf tests

  /** The resampling kernel this processor reads from; not owned. */
  const GOSoundResample &r_resample;

  /**
   * Which resampler (linear or polyphase) Process() uses; set at construction.
   */
  GOSoundResample::InterpolationType m_InterpolationType;

  /**
   * The maximum extra lag the read position may take on top of its minimum,
   * in milliseconds. Used only to size the delay ring in EnsureSetup(); set at
   * construction.
   */
  unsigned m_MaxLagMs;

  /** The length of the activation crossfade in milliseconds. */
  unsigned m_ActivationCrossfadeMs = DEFAULT_ACTIVATION_CROSSFADE_MS;

  /** The length of the deactivation crossfade in milliseconds. */
  unsigned m_DeactivationCrossfadeMs = DEFAULT_DEACTIVATION_CROSSFADE_MS;

  /** The channel count of the last EnsureSetup(). */
  unsigned m_NChannels = 0;

  /** The sample rate of the last EnsureSetup(). */
  unsigned m_SampleRate = 0;

  /** The period's frame count of the last EnsureSetup(); at least 2. */
  unsigned m_NFramesPerPeriod = 0;

  /**
   * The smallest allowed sum of the increments of a period: the one whose
   * mean rate is MIN_ALLOWED_RATE_UNITS. Set by EnsureSetup().
   */
  uint64_t m_MinAllowedIncrementPerPeriod = 0;

  /**
   * The largest allowed sum of the increments of a period: the one whose mean
   * rate is MAX_ALLOWED_RATE_UNITS. Set by EnsureSetup().
   */
  uint64_t m_MaxAllowedIncrementPerPeriod = 0;

  /**
   * The delay ring's size in frames, not counting the tail:
   * nMaxLagFrames + m_NFramesPerPeriod + N_LOOKAHEAD_FRAMES. The states are
   * created with it.
   */
  unsigned m_NDelayBufferRingFrames = 0;

  /** Whether EnsureSetup() has been called. */
  bool m_IsSetupCalled = false;

  /**
   * The increments of the resampler's position for the current audio period,
   * set by SetPitchRateChunk() before every Process(); a chunk with no rates
   * ({nullptr, 0, 0}) means bypass. Not owned: it points into a curve or into
   * a buffer of the one who sets it. Reset by EnsureSetup().
   */
  GOSoundVibratoPitchRateCurve::ChunkDescription m_PitchRateChunk
    = {nullptr, 0, 0};

  /**
   * The activation ramp: the weight of the undelayed signal falls 1 -> 0
   * (TO_OTHER). A constant-gain ramp (0: the delayed signal only) when the
   * crossfade is disabled. Built by EnsureSetup() and
   * SetActivationCrossfadeMs().
   */
  GOSoundLinearFaderCore::CrossfadeRamp m_ActivationRamp;

  /**
   * The deactivation ramp: the weight of the undelayed signal grows 0 -> 1
   * (FROM_OTHER). A constant-gain ramp (1: the undelayed signal only) when the
   * crossfade is disabled. Built by EnsureSetup() and
   * SetDeactivationCrossfadeMs().
   */
  GOSoundLinearFaderCore::CrossfadeRamp m_DeactivationRamp;

  /**
   * The same as the public EnsureSetup(), but all the lengths are given in
   * frames (0 is valid for each). The only place that sets the setup fields and
   * builds the ramps.
   * @param nMaxLagFrames the maximum extra lag in frames
   * @param nActivationFrames the length of the activation crossfade in frames;
   *   0 disables it
   * @param nDeactivationFrames the length of the deactivation crossfade in
   *   frames; 0 disables it
   * (the other parameters are those of the public EnsureSetup())
   */
  void EnsureSetup(
    unsigned nChannels,
    unsigned nFrames,
    unsigned sampleRate,
    unsigned nMaxLagFrames,
    unsigned nActivationFrames,
    unsigned nDeactivationFrames);

  /**
   * Returns a pointer to the increments of the resampler's position (in the
   * units of GOSoundResample, one per output frame) to use for this period: the
   * increments of the chunk itself if resampling with them keeps the
   * read position within the allowed range, otherwise a rescaled copy of them
   * in pTmpIncrements whose sum lands exactly on the violated bound.
   *
   * The final read position must be at least
   * max(nFrames - nFramesAvailableToWrite, 0) whole frames ahead, so that the
   * next write does not overwrite an unread frame, and at most
   * nFramesAvailableToRead ahead with a zero fraction, so that the last read
   * (before the last increment) has every interpolator tap written.
   *
   * Works on m_PitchRateChunk, which must have rates (HasRates()): its
   * m_NFramesPerPeriod increments and their sum (the advance of the read
   * position that resampling with them would make); read-only.
   *
   * @param readPosFraction the read position's current fraction
   * @param nFramesAvailableToRead state.GetNFramesAvailableToRead() after this
   *   period's write
   * @param nFramesAvailableToWrite state.GetNFramesAvailableToWrite() after
   *   this period's write
   * @param pTmpIncrements scratch space for m_NFramesPerPeriod increments;
   *   written only on a rescale
   * @return m_PitchRateChunk.pRateUnits itself if no rescale was needed,
   *   otherwise pTmpIncrements
   */
  const uint16_t *GetResamplerPositionIncrementsForUse(
    unsigned readPosFraction,
    unsigned nFramesAvailableToRead,
    unsigned nFramesAvailableToWrite,
    uint16_t *pTmpIncrements) const;

  /**
   * Resamples the delay buffer into outBuffer by the resampler of
   * m_InterpolationType and advances the state's read position. With
   * pPosIncrements the position advances by the given per-frame increments (the
   * chunk has rates); without them (nullptr) it advances at the constant rate
   * of the position itself, which the state keeps at 1.0 (the lag is frozen:
   * the read position advances as fast as the write one, its fraction is kept).
   *
   * @param state the processor state whose delay buffer is read and whose read
   *   position is advanced
   * @param pPosIncrements outBuffer.GetNFrames() increments of the resampler's
   *   position, as returned by GetResamplerPositionIncrementsForUse(), or
   *   nullptr for the constant rate 1.0
   * @param outBuffer the planar buffer to fill; it must have as many channels
   *   as the ring
   */
  void ReadFromDelayBuffer(
    GOSoundVibratoProcessorState &state,
    const uint16_t *pPosIncrements,
    GOSoundBufferPlanarMutable &outBuffer) const;

  /**
   * @param phase a phase of the state
   * @return whether the phase is not a crossfade: INACTIVE or ACTIVE
   */
  static inline bool isSteadyPhase(
    GOSoundVibratoProcessorState::ActivatingPhase phase) {
    return phase == GOSoundVibratoProcessorState::INACTIVE
      || phase == GOSoundVibratoProcessorState::ACTIVE;
  }

  /**
   * @param isToActivate whether the delayed signal is growing
   * @return the activation ramp if isToActivate, otherwise the deactivation one
   */
  inline const GOSoundLinearFaderCore::CrossfadeRamp &GetCrossfadeRamp(
    bool isToActivate) const {
    return isToActivate ? m_ActivationRamp : m_DeactivationRamp;
  }

  /**
   * Crossfades the undelayed signal with the delayed one for one period. The
   * ramps are the weights of the undelayed signal, the delayed one gets the
   * rest. A period may cross the end of the ramp: its remaining frames get the
   * constant final gain of the ramp.
   *
   * This is the only place that changes the phase and m_CrossfadePos of the
   * state (besides the resets): it starts a crossfade from INACTIVE / ACTIVE,
   * continues an unfinished crossfade whose direction or ramp length has
   * changed since the previous period (the position in the new ramp gives the
   * same gain), and moves the phase to ACTIVE or INACTIVE (resetting
   * m_CrossfadePos to 0) as soon as the ramp is over, even in the middle of the
   * period.
   *
   * Preconditions (asserted): delayedBuffer and inOutBuffer have the same
   * number of channels and frames and do not overlap; isToActivate requires the
   * phase to be not ACTIVE and !isToActivate requires it to be not INACTIVE -
   * those periods need no crossfade and must not be passed here.
   *
   * @param isToActivate true: the delayed signal grows (the chunk has rates),
   *   the activation ramp is used; false: it falls (no rates), the
   *   deactivation ramp is used
   * @param delayedBuffer the delayed signal of this period; not changed
   * @param state the processor state; the phase and m_CrossfadePos are updated
   * @param inOutBuffer in: the undelayed signal of this period; out: the
   *   crossfaded result
   */
  void CrossfadeWithDelayed(
    bool isToActivate,
    const GOSoundBufferPlanar &delayedBuffer,
    GOSoundVibratoProcessorState &state,
    GOSoundBufferPlanarMutable &inOutBuffer) const;

public:
  /**
   * Stores resample, interpolationType and maxLagMs.
   * @param resample the shared kernel instance this processor reads from for
   *   the lifetime of this object - see r_resample
   * @param interpolationType which resampler Process() uses for the lifetime
   *   of this object. Defaulted to the higher-quality polyphase one.
   * @param maxLagMs the maximum extra lag in milliseconds - see m_MaxLagMs. It
   *   sizes the delay ring of the states, so it cannot change later
   */
  explicit GOSoundVibratoProcessor(
    const GOSoundResample &resample,
    GOSoundResample::InterpolationType interpolationType
    = GOSoundResample::GO_POLYPHASE_INTERPOLATION,
    unsigned maxLagMs = DEFAULT_MAX_LAG_MS);

  /**
   * Sets the setup fields from the format (the delay ring covers the maximum
   * lag), builds the crossfade ramps from the current crossfade lengths and
   * resets the pitch rate chunk to "no rates": its owner sets it again with
   * SetPitchRateChunk() in every audio period.
   * @param nChannels see m_NChannels
   * @param nFrames the period's frame count, at least 2 - see
   *   m_NFramesPerPeriod
   * @param sampleRate see m_SampleRate
   */
  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override;

  /**
   * Sets the length of the activation crossfade. If EnsureSetup() has been
   * called, the ramp is rebuilt at once; otherwise the length is taken by the
   * next EnsureSetup(). The states need not be recreated: a crossfade that is
   * running continues with the same gain on the new ramp. Must not be called
   * concurrently with Process() (call it while the organ is suspended).
   * @param ms the length in milliseconds, at most MAX_CROSSFADE_MS; 0 disables
   *   the crossfade (the delayed signal is switched on at once)
   */
  void SetActivationCrossfadeMs(unsigned ms);

  /**
   * The same as SetActivationCrossfadeMs() for the deactivation crossfade.
   * @param ms the length in milliseconds, at most MAX_CROSSFADE_MS; 0 disables
   *   the crossfade (the delayed signal is switched off at once, with a click)
   */
  void SetDeactivationCrossfadeMs(unsigned ms);

  /**
   * Sets the increments of the resampler's position for the next Process()
   * calls, or the chunk with no rates for bypass. Called in every audio period
   * before the chain runs, so it is cheap: it only copies the three fields. It
   * is not synchronised: it must not be called concurrently with Process(). The
   * increments must stay valid until the next call. A chunk with rates must be
   * exactly m_NFramesPerPeriod (the nFrames of the last EnsureSetup()) long;
   * this is asserted.
   * @param chunk the increments and their sum, or {nullptr, 0, 0} to bypass
   */
  void SetPitchRateChunk(
    const GOSoundVibratoPitchRateCurve::ChunkDescription &chunk);

protected:
  /**
   * @return a new state sized for the last EnsureSetup(); asserts that
   *   EnsureSetup() was called
   */
  std::unique_ptr<GOSoundVibratoProcessorState> CreateTypedState()
    const override;

  /**
   * When the chunk has rates: writes the buffer into the delay buffer and reads
   * it back with the resampling rates of the chunk, rescaled if they would take
   * the read position out of its allowed range. From a state without the
   * delayed signal (INACTIVE) the output goes to the delayed signal by the
   * activation crossfade.
   *
   * When the chunk has no rates: if the delayed signal is still heard, the
   * output goes back to the undelayed one by the deactivation crossfade with
   * the frozen lag; after it, and when there was nothing to fade out, the
   * buffer is left untouched and the delay positions of the state are reset,
   * so the next activation starts at the minimum lag and the bypass costs no
   * latency.
   *
   * If the chunk gets rates again while the deactivation crossfade is running,
   * the crossfade turns back into an activation one without a jump. The lag
   * stays frozen at the value it had at the switch-off, not at the minimum. The
   * curve is normalised, so this extra lag (at most the lag at the switch-off)
   * does not grow; it is dropped at the next complete deactivation.
   */
  void Process(
    GOSoundVibratoProcessorState &state,
    GOSoundBufferPlanarMutable &buffer) const override;
};

#endif /* GOSOUNDVIBRATOPROCESSOR_H */
