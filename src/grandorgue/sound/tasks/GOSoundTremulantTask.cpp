/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include "sound/playing/GOSoundSamplerPlayer.h"

GOSoundTremulantTask::GOSoundTremulantTask(
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : GOSoundTaskBase(PRIORITY_TREMULANT, false),
    r_SamplerPlayer(samplerPlayer),
    m_volume(0),
    m_SamplesPerBuffer(nFramesPerBuffer) {}

void GOSoundTremulantTask::Add(GOSoundSampler *sampler) {
  m_Samplers.Put(sampler);
}

bool GOSoundTremulantTask::DoRun(GOSchedulerThread *pThread) {
  bool isDone = true;

  m_Samplers.Move();
  if (m_Samplers.Peek() == NULL)
    m_volume = 1;
  else {
    float outputBuffer[m_SamplesPerBuffer * 2];

    std::fill(outputBuffer, outputBuffer + m_SamplesPerBuffer * 2, 0.0f);
    outputBuffer[2 * m_SamplesPerBuffer - 1] = 1.0f;
    for (GOSoundSampler *sampler = m_Samplers.Get(); sampler;
         sampler = m_Samplers.Get()) {
      bool keep;
      keep = r_SamplerPlayer.ProcessSampler(
        outputBuffer, sampler, m_SamplesPerBuffer, 1);

      if (keep)
        m_Samplers.Put(sampler);
    }
    m_volume = outputBuffer[2 * m_SamplesPerBuffer - 1];
  }

  return isDone;
}
