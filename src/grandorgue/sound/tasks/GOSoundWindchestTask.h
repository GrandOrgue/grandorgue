/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTTASK_H
#define GOSOUNDWINDCHESTTASK_H

#include <atomic>

#include "model/GOWindchest.h"
#include "threading/GOMutex.h"

#include "GOSoundTaskBase.h"
#include "ptrvector.h"

class GOSchedulerThread;
class GOSoundOrganEngine;
class GOSoundTremulantTask;
class GOWindchest;

class GOSoundWindchestTask : public GOSoundTaskBase {
private:
  GOSoundOrganEngine &r_engine;
  GOMutex m_mutex;
  float m_amplitude;
  std::atomic_bool m_done;
  GOWindchest *p_windchest;
  std::vector<GOSoundTremulantTask *> m_pTremulantTasks;

public:
  GOSoundWindchestTask(
    GOSoundOrganEngine &sound_engine, GOWindchest *windchest);

  unsigned GetPriority() const override { return PRIORITY_WINDCHEST; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return false; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override {}

  void DiscardContent() override { NewRound(); }
  void NewRound() override;
  void Init(ptr_vector<GOSoundTremulantTask> &tremulantTasks);

  float GetWindchestAmplitude() const {
    return p_windchest ? p_windchest->GetAmplitude() : 1;
  }

  float GetAmplitude() {
    if (!m_done.load())
      Run();
    return m_amplitude;
  }
};

#endif
