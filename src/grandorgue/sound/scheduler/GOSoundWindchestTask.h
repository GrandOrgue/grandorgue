/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTTASK_H
#define GOSOUNDWINDCHESTTASK_H

#include <atomic>

#include "ptrvector.h"

#include "model/GOWindchest.h"
#include "sound/scheduler/GOSoundTask.h"
#include "threading/GOMutex.h"

class GOSoundEngine;
class GOSoundTremulantTask;
class GOWindchest;

class GOSoundWindchestTask : public GOSoundTask {
private:
  GOSoundEngine &r_engine;
  GOMutex m_mutex;
  float m_volume;
  std::atomic_bool m_done;
  GOWindchest *p_windchest;
  std::vector<GOSoundTremulantTask *> m_pTremulantTasks;

public:
  GOSoundWindchestTask(GOSoundEngine &sound_engine, GOWindchest *windchest);

  unsigned GetGroup() override { return WINDCHEST; }
  unsigned GetCost() override { return 0; }
  bool GetRepeat() override { return false; }
  void Run(GOSoundThread *pThread = nullptr) override;
  void Exec() override {}

  void Clear() override { Reset(); }
  void Reset() override;
  void Init(ptr_vector<GOSoundTremulantTask> &tremulantTasks);

  float GetWindchestVolume() const {
    return p_windchest ? p_windchest->GetVolume() : 1;
  }

  float GetVolume() {
    if (!m_done.load())
      Run();
    return m_volume;
  }
};

#endif
