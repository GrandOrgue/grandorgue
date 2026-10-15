/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDGROUPTASK_H
#define GOSOUNDGROUPTASK_H

#include <vector>

#include "GOSoundBufferTaskBase.h"

/**
 * Sums the buffers of one audio group's windchest-group tasks (see
 * GOSoundWindchestGroupTask) into its own buffer. Unlike
 * GOSoundWindchestGroupTask, this task does no cooperative per-sampler
 * mixing of its own: it treats its inputs as already-computed buffers, the
 * same way GOSoundOutputTask treats its audio-group inputs.
 */
class GOSoundGroupTask : public GOSoundBufferTaskBase {
private:
  /** The windchest-group tasks summed into this task's buffer, one per
   * (windchest, this audio group) pair that is actually used. Set by
   * SetInputs() during BuildEngine(); not owned by this task */
  std::vector<GOSoundBufferTaskBase *> mp_inputs;

  /** Sums mp_inputs into this task's buffer, see GOSoundTaskBase::DoRun() */
  bool DoRun(GOSchedulerThread *pThread) override;

public:
  /** @param nFramesPerBuffer the number of frames in this task's buffer */
  GOSoundGroupTask(unsigned nFramesPerBuffer);

  /** @return the windchest-group tasks summed into this task's buffer */
  const std::vector<GOSoundBufferTaskBase *> &GetInputs() const {
    return mp_inputs;
  }

  /** Sets the windchest-group tasks to sum into this task's buffer */
  void SetInputs(std::vector<GOSoundBufferTaskBase *> inputs);

  /** Forces the round to complete synchronously before returning, see
   * GOSoundOutputTask::EnsureBufferReady() */
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  /** Not cooperative, so a plain synchronous Run() suffices, see
   * GOSoundOutputTask::CompleteRound() */
  void CompleteRound() override { Run(); }
};

#endif
