/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTTASK_H
#define GOSOUNDWINDCHESTTASK_H

#include "model/GOWindchest.h"

#include "GOSoundTaskBase.h"
#include "ptrvector.h"

class GOSchedulerThread;
class GOSoundOrganEngine;
class GOSoundTremulantTask;
class GOWindchest;

class GOSoundWindchestTask : public GOSoundTaskBase {
private:
  GOSoundOrganEngine &r_engine;
  float m_volume;
  GOWindchest *p_windchest;
  std::vector<GOSoundTremulantTask *> m_pTremulantTasks;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  GOSoundWindchestTask(
    GOSoundOrganEngine &soundEngine, GOWindchest *pWindchest);

  void CompleteRound() override {}

  void Init(ptr_vector<GOSoundTremulantTask> &tremulantTasks);

  float GetWindchestVolume() const {
    return p_windchest ? p_windchest->GetVolume() : 1;
  }

  float GetVolume() {
    if (!IsDone())
      Run();
    return m_volume;
  }
};

#endif
