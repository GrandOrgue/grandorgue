/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestGroupTestFixture.h"

#include <cassert>

#include "sound/GOSoundDefs.h"
#include "sound/processing/GOSoundProcessingChain.h"
#include "sound/tasks/GOSoundWindchestGroupTask.h"

// The exact values only matter to a test that supplies a real chain via
// BuildWindchestTask(); every task this fixture's constructor pre-builds
// gets an empty chain, whose EnsureSetup() is a no-op.
static constexpr unsigned N_SAMPLES_PER_BUFFER = 32;
static constexpr unsigned SAMPLE_RATE = 96000;

// Shared by the constructor's pre-build loop and BuildWindchestTask():
// EnsureSetup()s pChain and hands it off to a freshly constructed
// GOSoundWindchestTask.
static std::unique_ptr<GOSoundWindchestTask> build_windchest_task(
  GOSoundOrganEngine &engine, std::unique_ptr<GOSoundProcessingChain> pChain) {
  pChain->EnsureSetup(MAX_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER, SAMPLE_RATE);

  return std::make_unique<GOSoundWindchestTask>(
    engine, nullptr, std::move(pChain));
}

GOSoundWindchestGroupTestFixture::GOSoundWindchestGroupTestFixture()
  : m_Config("GOSoundWindchestGroupTestFixture", ""),
    m_OrganModel(m_Config),
    m_Engine(m_OrganModel, m_MemoryPool),
    player(
      m_UnusedGrid,
      m_UnusedWindchestTasks,
      m_UnusedTremulantTasks,
      m_UnusedReleaseTask),
    roundCounter(m_Engine.GetScheduler().GetRoundCounter()) {
  for (unsigned windchestIndex = 0; windchestIndex < N_WINDCHEST_TASKS;
       windchestIndex++)
    m_WindchestTasksByIndex.push_back(build_windchest_task(
      m_Engine, std::make_unique<GOSoundProcessingChain>()));
}

GOSoundWindchestGroupTestFixture::~GOSoundWindchestGroupTestFixture() = default;

GOSoundWindchestTask &GOSoundWindchestGroupTestFixture::GetWindchestTask(
  unsigned windchestIndex) {
  assert(windchestIndex < m_WindchestTasksByIndex.size());

  return *m_WindchestTasksByIndex[windchestIndex];
}

GOSoundWindchestTask &GOSoundWindchestGroupTestFixture::BuildWindchestTask(
  std::unique_ptr<GOSoundProcessingChain> pChain) {
  m_BuiltWindchestTasks.push_back(
    build_windchest_task(m_Engine, std::move(pChain)));

  return *m_BuiltWindchestTasks.back();
}
