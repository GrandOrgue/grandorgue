/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDVIBRATOPROCESSOR_H
#define GOTESTSOUNDVIBRATOPROCESSOR_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundVibratoProcessor and GOSoundVibratoProcessorState: the
 * delay buffer geometry, the write into the ring, the resets, the choice of
 * the resampler position increments, the read back and the end-to-end
 * behaviour of Process() with curves built by GOSoundVibratoPitchRateCurve.
 * Friended by the processor, the state and the curve for direct access.
 */
class GOTestSoundVibratoProcessor : public GOTest {
private:
  static const std::string TEST_NAME;

  /** The constants and the sizes of a new state. */
  void TestStateConstructor();

  /** GetNFramesAvailableToWrite/Read() on the worked table. */
  void TestStateAvailableFrames();

  /** WriteToDelayBuffer(): the ring content, the mirrored tail, the write
   * position, over random positions and sizes, including the seam. */
  void TestStateWriteToDelayBuffer();

  /** Reset(): re-primes the positions, zeroes the look-ahead, keeps the rest
   * of the ring and the tail; harmless when repeated. */
  void TestStateReset();

  /** The phase and the crossfade position of a state: the initial values and
   * the resets. */
  void TestStatePhase();

  /** EnsureSetup(): the ring size, the chunk reset to "no rates". */
  void TestEnsureSetup();

  /** EnsureSetup(): the lengths and the directions of the crossfade ramps. */
  void TestEnsureSetupCrossfadeRamps();

  /** SetPitchRateChunk(): the chunk is stored, the chunk with no rates
   * bypasses. */
  void TestSetPitchRateChunk();

  /** CreateTypedState() sizes the state for the last EnsureSetup(). */
  void TestCreateTypedState();

  /** GetResamplerPositionIncrementsForUse() with a zero-width window. */
  void TestIncrementsZeroWidthWindow();

  /** GetResamplerPositionIncrementsForUse() with a wider window and the
   * bounds. */
  void TestIncrementsWiderWindow();

  /** GetResamplerPositionIncrementsForUse() on a chunk whose min target mean
   * falls below MIN_ALLOWED_RATE_UNITS. */
  void TestIncrementsLowerMeanBound();

  /** The same for the max target mean above MAX_ALLOWED_RATE_UNITS. */
  void TestIncrementsUpperMeanBound();

  /** GetResamplerPositionIncrementsForUse() on random chunks: the result
   * always lands inside the window and a no-violation chunk is returned as is.
   */
  void TestIncrementsRandom();

  /** ReadFromDelayBuffer(): a ramp across the ring seam. */
  void TestReadFromDelayBuffer();

  /** ReadFromDelayBuffer() without increments (the frozen lag) equals the
   * read with the unit increments and keeps the rate 1.0. */
  void TestReadFromDelayBufferFrozen();

  /** CrossfadeWithDelayed(): the phases, the weights, the direction changes,
   * the ramp end inside a period and the disabled crossfades. */
  void TestCrossfadeWithDelayed();

  /** Process() without a curve leaves the buffer untouched. */
  void TestProcessBypass();

  /** Process() with a zero-cents curve delays the signal by the look-ahead. */
  void TestProcessUnityCurveDelay();

  /** Process() with +1200 / +12 cents not normalised curves is rescaled onto
   * the exact delay. */
  void TestProcessRescaledToExactDelay();

  /** Process() with -1200 cents: the lag grows to its maximum and settles. */
  void TestProcessOctaveDownSettles();

  /** Process() with a normalised curve from Build() over several laps. */
  void TestProcessNormalisedCurve();

  /** Bypass after active periods and a restart rewind the state. */
  void TestProcessBypassAfterActiveAndRestart();

  /** Process() with the crossfades: the output on a unity curve, the phases
   * and the state after the deactivation. */
  void TestProcessCrossfade();

  /** Process() when the curve comes back during a deactivation: the lag stays
   * frozen while there is no curve, then the crossfade turns around without a
   * jump, the cursor of the curve starts from its head and the first chunk is
   * limited by the window of valid lags. */
  void TestProcessCrossfadeReactivation();

  /** Process() with the default crossfades: no jump of a sine at the switch-on
   * and the switch-off. */
  void TestProcessCrossfadeContinuity();

  /** Process() on a sine with a sine-shaped cents curve: the pitch of every
   * period of the output, measured by the zero crossings, follows the cents of
   * the curve within a few cents. */
  void TestProcessPitchDeviation();

  /** Garbage in the delay buffer never reaches the output. */
  void TestProcessGarbageNotLeaked();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDVIBRATOPROCESSOR_H */
