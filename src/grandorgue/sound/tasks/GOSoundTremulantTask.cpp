/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include "sound/playing/GOSoundSamplerPlayer.h"
#include "threading/GOMutexLocker.h"

GOSoundTremulantTask::GOSoundTremulantTask(
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : r_SamplerPlayer(samplerPlayer),
    m_Volume(0),
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
    m_Volume = 1;
    m_Done = true;
    return;
  }

  float output_buffer[m_SamplesPerBuffer * 2];
  std::fill(output_buffer, output_buffer + m_SamplesPerBuffer * 2, 0.0f);
  output_buffer[2 * m_SamplesPerBuffer - 1] = 1.0f;
  for (GOSoundSampler *sampler = m_Samplers.Get(); sampler;
       sampler = m_Samplers.Get()) {
    bool keep;
    keep = r_SamplerPlayer.ProcessSampler(
      output_buffer, sampler, m_SamplesPerBuffer, 1);

    if (keep)
      m_Samplers.Put(sampler);
  }
  m_Volume = output_buffer[2 * m_SamplesPerBuffer - 1];
  m_Done = true;
}
