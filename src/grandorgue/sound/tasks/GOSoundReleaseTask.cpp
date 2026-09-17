/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReleaseTask.h"

#include "sound/playing/GOSoundSamplerPlayer.h"

#include "GOSoundGroupTask.h"

GOSoundReleaseTask::GOSoundReleaseTask(
  GOSoundSamplerPlayer &samplerPlayer,
  ptr_vector<GOSoundGroupTask> &audioGroupTaskPtrs)
  : GOSoundTaskBase(PRIORITY_RELEASE, true),
    r_SamplerPlayer(samplerPlayer),
    r_AudioGroups(audioGroupTaskPtrs) {}

void GOSoundReleaseTask::DoNewRound() {
  m_Cnt.store(0);
  m_WaitCnt.store(0);
}

void GOSoundReleaseTask::Add(GOSoundSampler *sampler) { m_List.Put(sampler); }

/*
 * Unlike the other tasks, this one takes no mutex and never touches
 * m_RunState, so NewRound() may reset m_Cnt/m_WaitCnt and m_IsToComplete under
 * a worker that is still here. That is harmless because the task accumulates
 * no per-round result: there is no shared buffer to corrupt, IsDone() is never
 * read on it, and both counters are monotonic heuristics whose reset only
 * discards bookkeeping. Such a worker just carries on into the next period,
 * doing work that is already correct for it - NextPeriod() advances
 * m_CurrentTime before calling GOScheduler::NewRound().
 *
 * Note that the EnsureBufferReady() call below is the only path by which a
 * worker enters an audio group's round bypassing the scheduler's admission
 * control: GetNextTask() stops handing out tasks once the round's plan is
 * exhausted, and its counter is reset only at the end of
 * GOScheduler::NewRound(). A worker lingering here may therefore start a
 * group's next round before the rest of the pool is let in. That is safe
 * because GOSoundGroupTask::CompleteRound() leaves the group in the absorbing
 * RUN_STATE_DONE until its own NewRound(), so this call either returns at once
 * or claims a genuinely fresh round.
 */
void GOSoundReleaseTask::Run(GOSchedulerThread *pThread) {
  GOSoundSampler *sampler;

  do {
    while ((sampler = m_List.Get())) {
      m_Cnt.fetch_add(1);
      r_SamplerPlayer.ProcessRelease(sampler);
      if (m_IsToComplete.load() && m_Cnt > 10)
        break;
    }

    unsigned wait = m_WaitCnt.load();

    if (wait < r_AudioGroups.size()) {
      r_AudioGroups[wait]->EnsureBufferReady(false, pThread);
      m_WaitCnt.compare_exchange_strong(wait, wait + 1);
    }
  } while (!m_IsToComplete.load() && m_WaitCnt.load() < r_AudioGroups.size());
}

void GOSoundReleaseTask::CompleteRound() {
  GOSoundTaskBase::CompleteRound();

  GOSoundSampler *sampler;

  while ((sampler = m_List.Get()))
    r_SamplerPlayer.PassSampler(sampler);
}
