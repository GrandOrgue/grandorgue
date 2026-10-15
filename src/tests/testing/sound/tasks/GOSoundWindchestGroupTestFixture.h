/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTGROUPTESTFIXTURE_H
#define GOSOUNDWINDCHESTGROUPTESTFIXTURE_H

#include <memory>
#include <vector>

#include "config/GOConfig.h"
#include "model/GOOrganModel.h"
#include "scheduler/GORoundCounter.h"
#include "sound/GOSoundOrganEngine.h"
#include "sound/GOSoundWindchestGroupTaskGrid.h"
#include "sound/playing/GOSoundSamplerPlayer.h"
#include "sound/tasks/GOSoundReleaseTask.h"
#include "sound/tasks/GOSoundTremulantTask.h"
#include "sound/tasks/GOSoundWindchestTask.h"

#include "GOMemoryPool.h"
#include "ptrvector.h"

class GOSoundProcessingChain;

/**
 * A minimal, standalone fixture for tests that construct
 * GOSoundWindchestGroupTask or GOSoundWindchestGroupTaskGrid directly,
 * without a fully built sound engine. Shared between
 * GOTestSoundWindchestGroupTask.cpp and GOTestSoundWindchestGroupTaskGrid.cpp
 * (same convention as GOSoundCooperativeTaskTestImpl.h/.cpp in this
 * directory: a plain, non-GOTest-suffixed class included directly by every
 * test file that needs it).
 *
 * GOSoundWindchestGroupTask needs a real GOSoundWindchestTask
 * (r_WindchestTask), which needs a real GOSoundOrganEngine&, which needs a
 * real GOOrganModel& and GOMemoryPool& - but none of these need to be
 * built/loaded, only constructed: GOMemoryPool default-constructs with no
 * I/O, GOConfig's constructor does no filesystem I/O either (only in-memory
 * defaults), and GOOrganModel's constructor just stores its GOConfig& and
 * registers itself. BuildEngine()/StartEngine() are never called on
 * m_Engine, so this stays lightweight.
 */
class GOSoundWindchestGroupTestFixture {
private:
  GOMemoryPool m_MemoryPool;
  GOConfig m_Config;
  GOOrganModel m_OrganModel;
  GOSoundOrganEngine m_Engine;

  // Only exist to satisfy GOSoundSamplerPlayer's constructor references;
  // never populated by these tests, which build GOSoundWindchestGroupTask/
  // GOSoundWindchestGroupTaskGrid directly rather than going through the
  // player's own task lookup.
  GOSoundWindchestGroupTaskGrid m_UnusedGrid;
  std::vector<std::unique_ptr<GOSoundWindchestTask>> m_UnusedWindchestTasks;
  ptr_vector<GOSoundTremulantTask> m_UnusedTremulantTasks;
  std::unique_ptr<GOSoundReleaseTask> m_UnusedReleaseTask;

  /** One distinct GOSoundWindchestTask per windchestIndex in
   * [0, N_WINDCHEST_TASKS), all pre-built by the constructor
   * (GetWindchestTask() only reads this, never modifies it). Distinct indices
   * get distinct instances, matching GOSoundWindchestGroupTaskGrid::
   * BuildWindchestGroupTask()'s documented precondition that the task
   * passed for windchestIndex must be windchestIndex's own task. */
  static constexpr unsigned N_WINDCHEST_TASKS = 8;
  std::vector<std::unique_ptr<GOSoundWindchestTask>> m_WindchestTasksByIndex;

  /** Extra windchest tasks built ad hoc by BuildWindchestTask(), kept alive
   * for the fixture's lifetime; unlike m_WindchestTasksByIndex, not
   * addressed by index. */
  std::vector<std::unique_ptr<GOSoundWindchestTask>> m_BuiltWindchestTasks;

public:
  GOSoundSamplerPlayer player;
  /** m_Engine's own round counter, exposed the same way player is: tests
   * construct GOSoundWindchestGroupTask/GOSoundWindchestGroupTaskGrid
   * directly and need a real GORoundCounter for their constructors, but
   * m_Engine is never BuildEngine()'d so it has no populated scheduler of
   * its own to route through. */
  const GORoundCounter &roundCounter;

  GOSoundWindchestGroupTestFixture();

  // Defined in the .cpp: m_UnusedGrid's implicit destructor needs the
  // complete definition of GOSoundWindchestGroupTask, which this header
  // intentionally doesn't include (only forward-declared via
  // GOSoundWindchestGroupTaskGrid.h).
  ~GOSoundWindchestGroupTestFixture();

  /** @return the distinct GOSoundWindchestTask pre-built for windchestIndex
   * (see m_WindchestTasksByIndex); asserts windchestIndex < N_WINDCHEST_TASKS.
   */
  GOSoundWindchestTask &GetWindchestTask(unsigned windchestIndex);

  /**
   * Builds a new GOSoundWindchestTask from pChain (EnsureSetup() is called
   * on it here), kept alive for the fixture's lifetime. Unlike
   * GetWindchestTask(), which only ever returns one of the empty-chain tasks
   * pre-built by the constructor, this lets a test prove a specific
   * processor actually runs.
   * @param pChain the chain to build the task with; never null
   * @return the newly built task
   */
  GOSoundWindchestTask &BuildWindchestTask(
    std::unique_ptr<GOSoundProcessingChain> pChain);
};

#endif /* GOSOUNDWINDCHESTGROUPTESTFIXTURE_H */
