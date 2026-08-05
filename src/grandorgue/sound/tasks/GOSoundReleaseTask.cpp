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
