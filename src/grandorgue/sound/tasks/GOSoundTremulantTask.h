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

  /* DoRun() is a no-op (m_amplitude = 1, no sampler touched) whenever
     m_Samplers is empty - the exact same condition IsEmpty() above already
     tests. Unlike GOSoundWindchestTask/GOSoundTouchTask, this is not
     unconditional: reusing that predicate here means IsStateful() tracks
     whether the upcoming dispatch will actually process a sampler, not just
     whether this task type sometimes can (Codex review on PR #2620: "Avoid
     dirtying rounds for no-op stateful task runs") */
  bool IsStateful() const override { return !m_Samplers.IsEmpty(); }

  void DiscardContent() override { m_Samplers.Clear(); }
  void Add(GOSoundSampler *sampler);

  float GetAmplitude() {
    if (!IsDone())
      Run();
    return m_amplitude;
  }
};

#endif
