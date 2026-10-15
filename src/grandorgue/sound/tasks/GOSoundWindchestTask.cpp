/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestTask.h"

#include "sound/GOSoundOrganEngine.h"
#include "sound/processing/GOSoundProcessingChain.h"

#include "GOSoundTremulantTask.h"

GOSoundWindchestTask::GOSoundWindchestTask(
  GOSoundOrganEngine &soundEngine,
  GOWindchest *pWindchest,
  std::unique_ptr<GOSoundProcessingChain> pChain)
  : GOSoundTaskBase(PRIORITY_WINDCHEST, false),
    r_engine(soundEngine),
    m_amplitude(0),
    p_windchest(pWindchest),
    mp_chain(std::move(pChain)) {}

// The destructor body is empty (= default), but it must be defined here
// (not in the header) so that std::unique_ptr can call the complete
// destructor of its managed type (GOSoundProcessingChain), which is only
// forward-declared in the header file.
GOSoundWindchestTask::~GOSoundWindchestTask() = default;

void GOSoundWindchestTask::Init(
  ptr_vector<GOSoundTremulantTask> &tremulantTasks) {
  m_pTremulantTasks.clear();
  if (p_windchest)
    for (unsigned i = 0; i < p_windchest->GetTremulantCount(); i++)
      m_pTremulantTasks.push_back(
        tremulantTasks[p_windchest->GetTremulantId(i)]);
}

bool GOSoundWindchestTask::DoRun(GOSchedulerThread *pThread) {
  float amplitude = r_engine.GetAmplitude();

  if (p_windchest) {
    amplitude *= p_windchest->GetAmplitude();
    for (unsigned i = 0; i < m_pTremulantTasks.size(); i++)
      amplitude *= m_pTremulantTasks[i]->GetAmplitude();
  }
  m_amplitude = amplitude;
  mp_chain->EnsureParametersUpToDate();

  return true;
}
