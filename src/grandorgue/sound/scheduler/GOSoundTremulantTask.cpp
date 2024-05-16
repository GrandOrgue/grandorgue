/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include "sound/GOSoundEngine.h"
#include "threading/GOMutexLocker.h"

GOSoundTremulantTask::GOSoundTremulantTask(
  GOSoundEngine &sound_engine, unsigned samples_per_buffer)
  : m_engine(sound_engine),
    m_Volume(0),
    m_SamplesPerBuffer(samples_per_buffer),
    m_Done(false) {}

void GOSoundTremulantTask::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Done = false;
}

void GOSoundTremulantTask::Clear() { m_Samplers.Clear(); }

void GOSoundTremulantTask::Add(GOSoundSampler *sampler) {
  m_Samplers.Put(sampler);
}

unsigned GOSoundTremulantTask::GetGroup() { return TREMULANT; }

unsigned GOSoundTremulantTask::GetCost() { return 0; }

bool GOSoundTremulantTask::GetRepeat() { return false; }

void GOSoundTremulantTask::Run(GOSoundThread *thread) {
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
    keep
      = m_engine.ProcessSampler(output_buffer, sampler, m_SamplesPerBuffer, 1);

    if (keep)
      m_Samplers.Put(sampler);
  }
  m_Volume = output_buffer[2 * m_SamplesPerBuffer - 1];
  m_Done = true;
}

void GOSoundTremulantTask::Exec() { Run(); }
