/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDOUTPUTTASK_H
#define GOTESTSOUNDOUTPUTTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundOutputTask's IsEmpty()/DiscardContent() contract, and its
 * per-channel mixing, clamping and metering (Stage 3's planar rewrite of
 * GOSoundOutputTask::DoRun()), using a stub GOSoundBufferTaskBase input
 * instead of a real audio-group task.
 */
class GOTestSoundOutputTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A completed round that produced only silence, with reverb disabled,
   * must still make IsEmpty() false until NewRound() runs - meter and
   * reverb are silent, but the task is not GOSoundTaskBase::IsEmpty()
   * (not RUN_STATE_NOT_STARTED), and a caller must not treat a
   * done-but-not-reset task as safe to Add() back. */
  void TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound();

  /** DiscardContent() must leave the task IsEmpty(): besides resetting the
   * meter and reverb, it must also reset the round state (NewRound()),
   * or a task Run() once and then DiscardContent()'d would still fail
   * IsEmpty(). */
  void TestDiscardContentAfterRunMakesTaskEmpty();

  /** With an identity scale-factor matrix, each output channel equals the
   * corresponding input channel, unclamped values pass through unchanged. */
  void TestIdentityMixPassesThroughUnclamped();

  /** Values outside [-1, 1] are clamped in place, independently per
   * channel. */
  void TestClampsOutOfRangeValuesPerChannel();

  /** GetMeterInfo() reports the peak absolute clamped amplitude seen so
   * far, one entry per channel, and ResetMeterInfo() clears it back to 0. */
  void TestMeterInfoTracksPeakPerChannelAndResets();

  /** A zero scale factor excludes that input channel from the mix. */
  void TestZeroScaleFactorExcludesChannel();

  /** With a per-frame ramp (distinct value per channel and frame) as input,
   * every output frame of every channel must equal the corresponding input
   * value - catches channel/frame swap or stride bugs that a constant-value
   * fill checked only at frame 0 would miss. */
  void TestIdentityMixPreservesPerFrameLayout();

  /** DiscardContent() must reset the reverb engine, not just the meter: a
   * convolution tail left over from before deregistration must not bleed
   * into a round run after the task is Add()'d back without another
   * SetupReverb() call. */
  void TestDiscardContentResetsReverbTail();

  /** ResetMeterInfo() alone (called every period for the GUI meter, not
   * only on deregistration) must not make IsEmpty() report true while the
   * reverb engine may still hold an undecayed tail - only DiscardContent()
   * may clear that. */
  void TestIsEmptyStaysFalseAfterMeterResetWhileReverbActive();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDOUTPUTTASK_H */
