/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTTASKTESTFIXTURE_H
#define GOSOUNDTREMULANTTASKTESTFIXTURE_H

#include <memory>

#include "model/GOTremulant.h"
#include "sound/tasks/GOSoundTremulantTask.h"
#include "sound/tasks/GOSoundTremulantWaveForm.h"

#include "../../model/GOTestTremulantFixture.h"

/** The frames of one round in the tests of the tremulant task */
static constexpr unsigned TASK_TEST_N_FRAMES = 16;

/** The loop length in the tests of the tremulant task */
static constexpr unsigned TASK_TEST_N_LOOP = 64;

/**
 * The ramp lengths: not multiples of the round, so a ramp ends in the middle
 * of a round
 */
static constexpr unsigned TASK_TEST_N_START = 40;
static constexpr unsigned TASK_TEST_N_STOP = 28;

/** The peak of the amplitude deviation of the test loop */
static constexpr float TASK_TEST_AMP_DEPTH = 0.2f;

/** The peak of the pitch modulation of the test loop: 3 % of the frequency
 * (about 51 cents) */
static constexpr float TASK_TEST_PITCH_DEVIATION = 0.03f;

/** The sample rate passed to the task; the hand-made forms do not use it */
static constexpr unsigned TASK_TEST_SAMPLE_RATE = 48000;

/** A tremulant whose engagement can be set directly */
class GOTremulantTestImpl : public GOTremulant {
public:
  explicit GOTremulantTestImpl(GOOrganModel &organModel)
    : GOTremulant(organModel) {}

  void SetEngagedForTest(bool isEngaged) { m_IsEngaged = isEngaged; }
};

/**
 * A Synth tremulant with a task and a wave form installed in it. The form has
 * the test loop length, the test depths and the test ramps by default.
 */
struct GOSoundTremulantTaskTestFixture {
  GOTestTremulantModel model;
  GOTremulantTestImpl tremulant;
  GOSoundTremulantTask task;

  /**
   * Makes a wave form from a hand-made description with the round length of
   * the tests.
   *
   * @param nLoopFrames the length of the loop
   * @param ampDepth the peak amplitude deviation as a fraction
   * @param pitchDeviation the peak relative frequency deviation; 0 gives no
   *   pitch
   * @param nStartFrames the start ramp length
   * @param nStopFrames the stop ramp length
   */
  static std::unique_ptr<GOSoundTremulantWaveForm> MakeWaveForm(
    unsigned nLoopFrames,
    float ampDepth,
    float pitchDeviation,
    unsigned nStartFrames,
    unsigned nStopFrames);

  /** Starts a new round of the task and runs it */
  static void RunRound(GOSoundTremulantTask &task);

  /**
   * @param pitchDeviation the peak relative frequency deviation of the form;
   *   0 makes a form without pitch
   * @param nStartFrames the start ramp length
   * @param nStopFrames the stop ramp length
   * @param nLoopFrames the loop length
   */
  explicit GOSoundTremulantTaskTestFixture(
    float pitchDeviation = 0.0f,
    unsigned nStartFrames = TASK_TEST_N_START,
    unsigned nStopFrames = TASK_TEST_N_STOP,
    unsigned nLoopFrames = TASK_TEST_N_LOOP);

  /** Installs a wave form and drops the previous one */
  void InstallWaveForm(std::unique_ptr<GOSoundTremulantWaveForm> oWaveForm);

  /** Engages or disengages the tremulant */
  void SetEngaged(bool isEngaged) { tremulant.SetEngagedForTest(isEngaged); }

  /** Runs one round */
  void RunRound() { RunRound(task); }
};

#endif /* GOSOUNDTREMULANTTASKTESTFIXTURE_H */
