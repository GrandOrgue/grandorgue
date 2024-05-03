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
#include "sound/scheduler/GOSoundTask.h"
#include "threading/GOMutex.h"

class GOSoundEngine;
class GOSoundTremulantTask;
class GOWindchest;

class GOSoundWindchestTask : public GOSoundTask {
private:
  GOSoundEngine &m_engine;
  GOMutex m_Mutex;
  float m_Volume;
  std::atomic_bool m_Done;
  GOWindchest *m_Windchest;
  std::vector<GOSoundTremulantTask *> m_Tremulants;

public:
  GOSoundWindchestTask(GOSoundEngine &sound_engine, GOWindchest *windchest);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();
  void Init(ptr_vector<GOSoundTremulantTask> &tremulants);

  float GetWindchestVolume();
  float GetVolume() {
    if (!m_Done.load())
      Run();
    return m_Volume;
  }
};

#endif
