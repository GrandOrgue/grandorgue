/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundGroupTask.h"

#include <cassert>

#include "scheduler/GOSchedulerThread.h"

GOSoundGroupTask::GOSoundGroupTask(unsigned nFramesPerBuffer)
  : GOSoundBufferTaskBase(PRIORITY_AUDIOGROUP, false, 2, nFramesPerBuffer),
    mp_inputs() {}

void GOSoundGroupTask::SetInputs(std::vector<GOSoundBufferTaskBase *> inputs) {
  mp_inputs = std::move(inputs);
}

bool GOSoundGroupTask::DoRun(GOSchedulerThread *pThread) {
  bool isStopped = false;
  const unsigned n = mp_inputs.size();

  if (n) {
    for (unsigned i = 0; i < n && !isStopped; i++) {
      GOSoundBufferTaskBase *input = mp_inputs[i];

      input->EnsureBufferReady(m_IsToComplete.load(), pThread);
      if (pThread && pThread->ShouldStop())
        isStopped = true;
      else if (i == 0)
        CopyFrom(*input);
      else
        AddFrom(*input);
    }
  } else
    FillWithSilence();

  return !isStopped;
}

void GOSoundGroupTask::EnsureBufferReady(
  bool isToComplete, GOSchedulerThread *pThread) {
  if (isToComplete)
    m_IsToComplete.store(true);
  if (!IsDone())
    Run(pThread);
}
