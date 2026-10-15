/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTTASK_H
#define GOSOUNDWINDCHESTTASK_H

#include <memory>

#include "model/GOWindchest.h"

#include "GOSoundTaskBase.h"
#include "ptrvector.h"

class GOSchedulerThread;
class GOSoundOrganEngine;
class GOSoundProcessingChain;
class GOSoundTremulantTask;
class GOWindchest;

class GOSoundWindchestTask : public GOSoundTaskBase {
private:
  GOSoundOrganEngine &r_engine;
  float m_amplitude;
  GOWindchest *p_windchest;
  std::vector<GOSoundTremulantTask *> m_pTremulantTasks;
  /** This windchest's processing chain, shared by every
   * GOSoundWindchestGroupTask (audio group) of this windchest. Received
   * already built and EnsureSetup() at construction (see the constructor's
   * pChain parameter); never null. */
  std::unique_ptr<GOSoundProcessingChain> mp_chain;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  /**
   * @param soundEngine the owning engine, used for its overall amplitude
   * @param pWindchest the model windchest this task tracks, or nullptr for
   *   the synthetic detached-release windchest
   * @param pChain this windchest's processing chain, taken over by this
   *   task; never null (an unregistered windchest still gets a valid,
   *   empty chain, built by the caller), and already EnsureSetup() by the
   *   caller
   */
  GOSoundWindchestTask(
    GOSoundOrganEngine &soundEngine,
    GOWindchest *pWindchest,
    std::unique_ptr<GOSoundProcessingChain> pChain);
  ~GOSoundWindchestTask();

  void CompleteRound() override {}

  void Init(ptr_vector<GOSoundTremulantTask> &tremulantTasks);

  float GetWindchestAmplitude() const {
    return p_windchest ? p_windchest->GetAmplitude() : 1;
  }

  float GetAmplitude() {
    if (!IsDone())
      Run();
    return m_amplitude;
  }

  /** @return this windchest's processing chain; never null (an unregistered
   * windchest still gets a valid, empty chain). */
  const GOSoundProcessingChain &GetChain() const { return *mp_chain; }
};

#endif
