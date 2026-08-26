/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReleaseTask.h"

#include "GOSoundGroupTask.h"
#include "sound/playing/GOSoundSamplerPlayer.h"

GOSoundReleaseTask::GOSoundReleaseTask(
  GOSoundSamplerPlayer &samplerPlayer,
  ptr_vector<GOSoundGroupTask> &audioGroupTaskPtrs)
  : r_SamplerPlayer(samplerPlayer),
    m_AudioGroups(audioGroupTaskPtrs),
    m_IsToComplete(false) {}

void GOSoundReleaseTask::NewRound() {
  m_IsToComplete.store(false);
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
    if (wait < m_AudioGroups.size()) {
      m_AudioGroups[wait]->EnsureBufferReady(false, pThread);
      m_WaitCnt.compare_exchange_strong(wait, wait + 1);
    }
  } while (!m_IsToComplete.load() && m_WaitCnt.load() < m_AudioGroups.size());
}

void GOSoundReleaseTask::CompleteRound() {
  m_IsToComplete.store(true);
  Run();
  GOSoundSampler *sampler;
  while ((sampler = m_List.Get()))
    r_SamplerPlayer.PassSampler(sampler);
}
