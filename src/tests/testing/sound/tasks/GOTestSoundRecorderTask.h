/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDRECORDERTASK_H
#define GOTESTSOUNDRECORDERTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundRecorderTask::ConvertData<T>()'s gather logic (Stage 3's
 * planar rewrite: it now reads each output task's channels contiguously
 * instead of walking one interleaved buffer), using stub GOSoundBufferTaskBase
 * inputs instead of real audio-group tasks.
 */
class GOTestSoundRecorderTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** Records one buffer's worth of float samples from two output tasks with
   * different channel counts and verifies the WAV file's interleaved data
   * matches, frame by frame and channel by channel, in the order
   * SetOutputs() was given - the exact start_pos/m_Channels-stride path
   * ConvertData<T>() inverted for Stage 3. */
  void TestGathersDistinctChannelCountsInOrder();

  /** DiscardContent() must close an open recording, not just reset the
   * round: a task deregistered mid-recording and later Add()'d back must
   * start a fresh recording rather than keep appending to the old file. */
  void TestDiscardContentClosesOpenRecording();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDRECORDERTASK_H */
