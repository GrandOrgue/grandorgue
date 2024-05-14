/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestTask.h"

#include "sound/GOSoundEngine.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundTremulantTask.h"

GOSoundWindchestTask::GOSoundWindchestTask(
  GOSoundEngine &soundEngine, GOWindchest *pWindchest)
  : r_engine(soundEngine),
    m_volume(0),
    m_done(false),
    p_windchest(pWindchest) {}

void GOSoundWindchestTask::Init(
  ptr_vector<GOSoundTremulantTask> &tremulantTasks) {
  m_pTremulantTasks.clear();
  if (p_windchest)
    for (unsigned i = 0; i < p_windchest->GetTremulantCount(); i++)
      m_pTremulantTasks.push_back(
        tremulantTasks[p_windchest->GetTremulantId(i)]);
}

void GOSoundWindchestTask::Reset() {
  GOMutexLocker locker(m_mutex);

  m_done.store(false);
}

void GOSoundWindchestTask::Run(GOSoundThread *pThread) {
  if (!m_done.load()) {
    GOMutexLocker locker(m_mutex);

    if (!m_done.load()) {
      float volume = r_engine.GetGain();

      if (p_windchest) {
        volume *= p_windchest->GetVolume();
        for (unsigned i = 0; i < m_pTremulantTasks.size(); i++)
          volume *= m_pTremulantTasks[i]->GetVolume();
      }
      m_volume = volume;
      m_done.store(true);
    }
  }
}
