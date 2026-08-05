/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestTask.h"

#include "sound/GOSoundOrganEngine.h"

#include "GOSoundTremulantTask.h"

GOSoundWindchestTask::GOSoundWindchestTask(
  GOSoundOrganEngine &soundEngine, GOWindchest *pWindchest)
  : GOSoundTaskBase(PRIORITY_WINDCHEST, false),
    r_engine(soundEngine),
    m_volume(0),
    p_windchest(pWindchest) {}

void GOSoundWindchestTask::Init(
  ptr_vector<GOSoundTremulantTask> &tremulantTasks) {
  m_pTremulantTasks.clear();
  if (p_windchest)
    for (unsigned i = 0; i < p_windchest->GetTremulantCount(); i++)
      m_pTremulantTasks.push_back(
        tremulantTasks[p_windchest->GetTremulantId(i)]);
}

bool GOSoundWindchestTask::DoRun(GOSchedulerThread *pThread) {
  float volume = r_engine.GetGain();

  if (p_windchest) {
    volume *= p_windchest->GetVolume();
    for (unsigned i = 0; i < m_pTremulantTasks.size(); i++)
      volume *= m_pTremulantTasks[i]->GetVolume();
  }
  m_volume = volume;

  return true;
}
