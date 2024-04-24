/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReleaseWorkItem.h"

#include "GOSoundGroupWorkItem.h"
#include "sound/GOSoundEngine.h"

GOSoundReleaseWorkItem::GOSoundReleaseWorkItem(
  GOSoundEngine &sound_engine, ptr_vector<GOSoundGroupWorkItem> &audio_groups)
  : m_engine(sound_engine), m_AudioGroups(audio_groups), m_Stop(false) {}

unsigned GOSoundReleaseWorkItem::GetGroup() { return RELEASE; }

unsigned GOSoundReleaseWorkItem::GetCost() { return 0; }

bool GOSoundReleaseWorkItem::GetRepeat() { return true; }

void GOSoundReleaseWorkItem::Clear() { m_List.Clear(); }

void GOSoundReleaseWorkItem::Reset() {
  m_Stop.store(false);
  m_Cnt.store(0);
  m_WaitCnt.store(0);
}

void GOSoundReleaseWorkItem::Add(GOSoundSampler *sampler) {
  m_List.Put(sampler);
}

void GOSoundReleaseWorkItem::Run(GOSoundThread *pThread) {
  GOSoundSampler *sampler;
  do {
    while ((sampler = m_List.Get())) {
      m_Cnt.fetch_add(1);
      m_engine.ProcessRelease(sampler);
      if (m_Stop.load() && m_Cnt > 10)
        break;
    }
    unsigned wait = m_WaitCnt.load();
    if (wait < m_AudioGroups.size()) {
      m_AudioGroups[wait]->Finish(false, pThread);
      m_WaitCnt.compare_exchange_strong(wait, wait + 1);
    }
  } while (!m_Stop.load() && m_WaitCnt.load() < m_AudioGroups.size());
}

void GOSoundReleaseWorkItem::Exec() {
  m_Stop.store(true);
  Run();
  GOSoundSampler *sampler;
  while ((sampler = m_List.Get()))
    m_engine.PassSampler(sampler);
}
