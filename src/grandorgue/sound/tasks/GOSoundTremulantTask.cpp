/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundSamplerPlayer.h"
#include "threading/GOMutexLocker.h"

GOSoundTremulantTask::GOSoundTremulantTask(
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : r_SamplerPlayer(samplerPlayer),
    m_amplitude(0),
    m_SamplesPerBuffer(nFramesPerBuffer),
    m_Done(false) {}

void GOSoundTremulantTask::NewRound() {
  GOMutexLocker locker(m_Mutex);
  m_Done = false;
}

void GOSoundTremulantTask::Add(GOSoundSampler *sampler) {
  m_Samplers.Put(sampler);
}

void GOSoundTremulantTask::Run(GOSchedulerThread *pThread) {
  if (m_Done)
    return;

  GOMutexLocker locker(m_Mutex);

  if (m_Done)
    return;

  m_Samplers.Move();
  if (m_Samplers.Peek() == NULL) {
    m_amplitude = 1;
    m_Done = true;
    return;
  }

  GO_DECLARE_LOCAL_SOUND_BUFFER(outputBuffer, 2, m_SamplesPerBuffer)

  outputBuffer.FillWithSilence();
  outputBuffer.GetData()[2 * m_SamplesPerBuffer - 1] = 1.0f;
  for (GOSoundSampler *pSampler = m_Samplers.Get(); pSampler;
       pSampler = m_Samplers.Get()) {
    if (r_SamplerPlayer.ProcessSampler(*pSampler, 1.0f, outputBuffer))
      m_Samplers.Put(pSampler);
  }
  m_amplitude = outputBuffer.GetData()[2 * m_SamplesPerBuffer - 1];
  m_Done = true;
}
