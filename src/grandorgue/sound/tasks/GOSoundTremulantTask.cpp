/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundSamplerPlayer.h"

GOSoundTremulantTask::GOSoundTremulantTask(
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : GOSoundTaskBase(PRIORITY_TREMULANT, false),
    r_SamplerPlayer(samplerPlayer),
    m_amplitude(0),
    m_SamplesPerBuffer(nFramesPerBuffer) {}

void GOSoundTremulantTask::Add(GOSoundSampler *sampler) {
  m_Samplers.Put(sampler);
}

bool GOSoundTremulantTask::DoRun(GOSchedulerThread *pThread) {
  m_Samplers.Move();
  if (m_Samplers.Peek() == NULL)
    m_amplitude = 1;
  else {
    GO_DECLARE_LOCAL_SOUND_BUFFER(outputBuffer, 2, m_SamplesPerBuffer)

    outputBuffer.FillWithSilence();
    outputBuffer.GetData()[2 * m_SamplesPerBuffer - 1] = 1.0f;
    for (GOSoundSampler *sampler = m_Samplers.Get(); sampler;
         sampler = m_Samplers.Get()) {
      if (r_SamplerPlayer.ProcessSampler(*sampler, 1.0f, outputBuffer))
        m_Samplers.Put(sampler);
    }
    m_amplitude = outputBuffer.GetData()[2 * m_SamplesPerBuffer - 1];
  }

  return true;
}
