/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTTASK_H
#define GOSOUNDTREMULANTTASK_H

#include "sound/playing/GOSoundSamplerList.h"
#include "threading/GOMutex.h"

#include "GOSoundTaskBase.h"

class GOSchedulerThread;
class GOSoundSamplerPlayer;

class GOSoundTremulantTask : public GOSoundTaskBase {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
  GOSoundSamplerList m_Samplers;
  GOMutex m_Mutex;
  float m_amplitude;
  unsigned m_SamplesPerBuffer;
  bool m_Done;

public:
  GOSoundTremulantTask(
    GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer);

  unsigned GetPriority() const override { return PRIORITY_TREMULANT; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return false; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override { Run(); }

  void NewRound() override;
  void DiscardContent() override { m_Samplers.Clear(); }
  void Add(GOSoundSampler *sampler);

  float GetAmplitude() {
    if (!m_Done)
      Run();
    return m_amplitude;
  }
};

#endif
