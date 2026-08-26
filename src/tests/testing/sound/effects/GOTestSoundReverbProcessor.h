/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDREVERBPROCESSOR_H
#define GOTESTSOUNDREVERBPROCESSOR_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundReverbProcessor/GOSoundReverbProcessorState (Stage 5's
 * sound/effects/ smoke-test processor) against the same real WAV fixture as
 * GOTestSoundReverb (resources/sound/reverb/test-ir.wav). Friended by both
 * classes for direct field access, matching sound/processing/'s own test
 * suites (e.g. GOTestSoundProcessingChain).
 */
class GOTestSoundReverbProcessor : public GOTest {
private:
  static const std::string TEST_NAME;

  /** CreateTypedState() builds exactly one Convproc per channel requested
   * by EnsureSetup(). */
  void TestCreateTypedStateBuildsOneConvprocPerChannel();

  /** A later EnsureSetup() call with a different channel count reconfigures
   * - the next CreateTypedState() reflects the new count, not the old one. */
  void TestEnsureSetupReloadsOnFormatChange();

  /** Process(), called through the untyped GOSoundProcessor entry point (the
   * same path the real chain framework uses), runs without crashing across
   * several rounds, leaves the buffer's shape (channel/frame count)
   * unchanged, and actually changes the audio content by the last round -
   * exact convolution output isn't asserted, since zita-convolver introduces
   * processing latency that makes bit-exact output timing-dependent, but a
   * silent bypass (e.g. from an unbuilt/no-op state) must be caught. */
  void TestProcessRunsAcrossSeveralRounds();

  /** Reset() can be called both before and after Process(), without
   * crashing. */
  void TestResetDoesNotCrash();

  /** A missing impulse-response file leaves EnsureSetup()'s load unsuccessful
   * (m_IsIRLoaded stays false); CreateTypedState() must still build a valid
   * (though no-op) state - an empty mp_ConvprocsByChannel, not one Convproc
   * per channel - rather than starting engines with nothing to convolve.
   * Process() on that state must be a silent bypass, not a crash. */
  void TestMissingIrFileBuildsNoOpState();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDREVERBPROCESSOR_H */
