/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestWorkItem.h"

#include "model/GOWindchest.h"
#include "sound/GOSoundEngine.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundTremulantWorkItem.h"

GOSoundWindchestWorkItem::GOSoundWindchestWorkItem(
  GOSoundEngine &sound_engine, GOWindchest *windchest)
  : m_engine(sound_engine),
    m_Volume(0),
    m_Done(false),
    m_Windchest(windchest),
    m_Tremulants() {}

void GOSoundWindchestWorkItem::Init(
  ptr_vector<GOSoundTremulantWorkItem> &tremulants) {
  m_Tremulants.clear();
  if (!m_Windchest)
    return;
  for (unsigned i = 0; i < m_Windchest->GetTremulantCount(); i++)
    m_Tremulants.push_back(tremulants[m_Windchest->GetTremulantId(i)]);
}

void GOSoundWindchestWorkItem::Clear() { Reset(); }

void GOSoundWindchestWorkItem::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Done.store(false);
}

unsigned GOSoundWindchestWorkItem::GetGroup() { return WINDCHEST; }

unsigned GOSoundWindchestWorkItem::GetCost() { return 0; }

bool GOSoundWindchestWorkItem::GetRepeat() { return false; }

float GOSoundWindchestWorkItem::GetWindchestVolume() {
  if (m_Windchest != NULL)
    return m_Windchest->GetVolume();
  else
    return 1;
}

void GOSoundWindchestWorkItem::Run(GOSoundThread *thread) {
  if (m_Done.load())
    return;

  GOMutexLocker locker(m_Mutex);

  if (m_Done.load())
    return;

  float volume = m_engine.GetGain();
  if (m_Windchest != NULL) {
    volume *= m_Windchest->GetVolume();
    for (unsigned i = 0; i < m_Tremulants.size(); i++)
      volume *= m_Tremulants[i]->GetVolume();
  }
  m_Volume = volume;
  m_Done.store(true);
}

void GOSoundWindchestWorkItem::Exec() {}
