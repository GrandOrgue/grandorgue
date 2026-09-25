/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestGroupTestFixture.h"

#include "sound/tasks/GOSoundWindchestGroupTask.h"

GOSoundWindchestGroupTestFixture::GOSoundWindchestGroupTestFixture()
  : m_Config("GOSoundWindchestGroupTestFixture", ""),
    m_OrganModel(m_Config),
    m_Engine(m_OrganModel, m_MemoryPool),
    player(
      m_UnusedGrid,
      m_UnusedWindchestTasks,
      m_UnusedTremulantTasks,
      m_UnusedReleaseTask),
    roundCounter(m_Engine.GetScheduler().GetRoundCounter()) {}

GOSoundWindchestGroupTestFixture::~GOSoundWindchestGroupTestFixture() = default;

GOSoundWindchestTask &GOSoundWindchestGroupTestFixture::GetWindchestTask(
  unsigned windchestIndex) {
  while (m_WindchestTasksByIndex.size() <= windchestIndex)
    m_WindchestTasksByIndex.push_back(
      std::make_unique<GOSoundWindchestTask>(m_Engine, nullptr));

  return *m_WindchestTasksByIndex[windchestIndex];
}
