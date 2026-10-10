/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSHELFFILTERPROCESSOR_H
#define GOSOUNDSHELFFILTERPROCESSOR_H

#include "sound/dsp-kernels/GOSoundOnePoleFilter.h"
#include "sound/processing/GOSoundProcessorTyped.h"

#include "GOSoundShelfFilterProcessorState.h"

/**
 * Two-band shelf EQ sound effect for a windchest's processing chain: a low
 * shelf and a high shelf applied in series to every channel of the buffer.
 * Built for swell-box modeling (GrandOrgue issue #717) - each band's
 * frequency/gain is expected to be updated every round by a future
 * enclosure-position mapper via SetLowShelf()/SetHighShelf(), not just once
 * at setup. Uses only GOSoundOnePoleFilter's static building blocks
 * (Coeffs/computeCoeffs()/processSample()), not its instance API.
 */
class GOSoundShelfFilterProcessor
  : public GOSoundProcessorTyped<GOSoundShelfFilterProcessorState> {
private:
  friend class GOTestSoundShelfFilterProcessor; // direct access for unit
                                                // tests

  double m_LowFrequency = 0, m_LowGain = 0;
  double m_HighFrequency = 0, m_HighGain = 0;
  GOSoundOnePoleFilter::Coeffs m_LowCoeffs;
  GOSoundOnePoleFilter::Coeffs m_HighCoeffs;
  unsigned m_NChannels = 0;
  unsigned m_SampleRate = 0;
  bool m_IsSetupCalled = false;

protected:
  /** @return a new GOSoundShelfFilterProcessorState sized for the channel
   * count from the most recent EnsureSetup() call; asserts EnsureSetup()
   * was already called. */
  std::unique_ptr<GOSoundShelfFilterProcessorState> CreateTypedState()
    const override;

  /**
   * Applies the low shelf then the high shelf, in series, to every channel
   * of buffer, each via GOSoundOnePoleFilter::processSample(). Skips a
   * band entirely when its Coeffs.isActive is false, and returns immediately
   * without touching buffer at all when both bands are not isActive. A band
   * that has just become inactive gets its state cleared.
   */
  void Process(
    GOSoundShelfFilterProcessorState &state,
    GOSoundBufferPlanarMutable &buffer) const override;

public:
  /** Leaves both bands at the identity (not isActive) default - zero Process()
   * cost until a caller configures them via SetLowShelf()/SetHighShelf().
   * Deliberately takes no initial-parameter arguments: parameters are
   * inherently dynamic (mapper-driven), so a constructor-supplied initial
   * value could never be heard once a mapper is attached, and would just
   * be a second, redundant way to set what SetLowShelf()/SetHighShelf()
   * already set. */
  GOSoundShelfFilterProcessor() = default;

  /** Recomputes both bands' Coeffs (from the current frequency/gain
   * fields) only when sampleRate actually changes; stores nChannels for
   * CreateTypedState(). Cheap no-op otherwise, per
   * GOSoundProcessor::EnsureSetup()'s contract. */
  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override;

  /**
   * Updates the low shelf band's frequency/gain and recomputes its
   * coefficients. Expensive: recomputes cos/sin/pow every call, so this is
   * not free to call every round on an unchanged value - callers (e.g. a
   * future enclosure-position mapper) must cache the last value they
   * pushed and call this only when the parameter actually changed enough
   * to matter (smoothing/dead-banding is the caller's responsibility, not
   * this processor's). Kept independent from SetHighShelf() so a caller
   * can skip recomputing whichever band didn't change.
   * @param frequency Low shelf corner frequency, in Hz
   * @param gain Low shelf gain in dB; 0 makes this band a no-op
   */
  void SetLowShelf(double frequency, double gain);

  /** Same contract and cost as SetLowShelf(), for the high shelf band.
   * @param frequency High shelf corner frequency, in Hz
   * @param gain High shelf gain in dB; 0 makes this band a no-op */
  void SetHighShelf(double frequency, double gain);
};

#endif /* GOSOUNDSHELFFILTERPROCESSOR_H */
