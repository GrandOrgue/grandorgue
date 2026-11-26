/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTREMULANTTASK_H
#define GOTESTSOUNDTREMULANTTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundTremulantTask: the amplitude, the depth ramps, the cursor,
 * replacing the wave form and building it from a tremulant. It is a friend of
 * the task and reads its state directly. The pitch rates are in
 * GOTestSoundTremulantTaskPitch.
 */
class GOTestSoundTremulantTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A Wave tremulant has no wave form, amplitude 1.0 and no buffers. */
  void TestWaveTremulant();

  /** A disengaged tremulant gives 1.0 and the cursor stays. */
  void TestIdleState();

  /** Engaging ramps the depth up over the start time. */
  void TestRampUp();

  /** Disengaging ramps the depth down and resets the cursor at the end. */
  void TestRampDown();

  /** Engaging continues from the current phase, it is not reset. */
  void TestStartFromAnyPosition();

  /** Reversing a ramp keeps the depth. */
  void TestReversalMidRamp();

  /** Ramps of 0 frames switch the depth instantly. */
  void TestInstantStartStop();

  /** The cursor follows the sum of the gains, with no drift. */
  void TestCursorFollowsGainSum();

  /** EnsureDone() runs the task once per round. */
  void TestEnsureDone();

  /** SwapWaveForm() keeps the phase and the depth. */
  void TestSwapWaveForm();

  /**
   * A swap onto a wave form whose falling ramp is already over, made during a
   * falling ramp, resets the cursor to the loop start.
   */
  void TestSwapOntoInstantStopDuringFallingRamp();

  /** No inf or NaN with ramps longer and shorter than a round. */
  void TestNoNaN();

  /** BuildWaveForm() renders the tremulant. */
  void TestBuildWaveForm();

  /** The buffers are not reallocated while running and swapping. */
  void TestStableBuffers();

  /** A form without amplitude keeps the gain 1.0, the pitch works. */
  void TestNoAmplitude();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTTASK_H */
