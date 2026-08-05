/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTTASK_H
#define GOSOUNDTREMULANTTASK_H

#include "sound/playing/GOSoundSamplerList.h"

#include "GOSoundTaskBase.h"

class GOSchedulerThread;
class GOSoundSamplerPlayer;

class GOSoundTremulantTask : public GOSoundTaskBase {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
  GOSoundSamplerList m_Samplers;
  float m_amplitude;
  unsigned m_SamplesPerBuffer;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  GOSoundTremulantTask(
    GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer);

  bool IsEmpty() const override { return m_Samplers.IsEmpty(); }

  void DiscardContent() override { m_Samplers.Clear(); }
  void Add(GOSoundSampler *sampler);

  float GetAmplitude() {
    if (!IsDone())
      Run();
    return m_amplitude;
  }
};

#endif
