/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTGROUPTASKGRID_H
#define GOSOUNDWINDCHESTGROUPTASKGRID_H

#include <cassert>
#include <memory>
#include <vector>

class GORoundCounter;
class GOSoundBufferTaskBase;
class GOSoundSamplerPlayer;
class GOSoundWindchestGroupTask;
class GOSoundWindchestTask;

/**
 * Owns every GOSoundWindchestGroupTask, addressed by (windchestIndex,
 * audioGroupId), for the sound engine. Only pairs that actually occur among
 * the organ's pipes are constructed (see
 * GOOrganModel::GetUsedWindchestGroupPairs()), but addressing stays dense: a
 * single flat, row-major vector of pointers gives O(1) lookup with no scan and
 * no hashing, at negligible memory cost even when mostly unused (windchestIndex
 * 0 is the detached-release windchest, same indexing as GOSoundOrganEngine's
 * windchest task list).
 *
 * This class owns every constructed task; nothing else does, matching how
 * GOSoundOrganEngine owns every other sound task as a flat collection while
 * tasks only ever hold non-owning pointers to each other.
 */
class GOSoundWindchestGroupTaskGrid {
private:
  /** The audio group count used to size m_cells and compute GetIndex() */
  unsigned m_NAudioGroups = 0;

  /** Row-major [windchestIndex][audioGroupId] grid of owned tasks; a null
   * cell means that pair is not used by any pipe */
  std::vector<std::unique_ptr<GOSoundWindchestGroupTask>> m_cells;

  /** @return the row-major index of a (windchestIndex, audioGroupId) cell */
  unsigned GetIndex(unsigned windchestIndex, unsigned audioGroupId) const {
    assert(audioGroupId < m_NAudioGroups);

    const unsigned index = windchestIndex * m_NAudioGroups + audioGroupId;

    assert(index < m_cells.size());

    return index;
  }

  /**
   * @return the task at (windchestIndex, audioGroupId), or nullptr if that
   *   pair is not used. For internal use only: GetInputsForGroup() uses this
   *   to tolerate the common case of an unused pair; external callers that
   *   know the pair must exist should use GetWindchestGroupTask() instead.
   */
  GOSoundWindchestGroupTask *FindWindchestGroupTask(
    unsigned windchestIndex, unsigned audioGroupId) const;

public:
  /**
   * Allocates the grid, all cells null. Must be called once before any other
   * method.
   * @param nWindchests the number of windchest task slots (including the
   *   detached-release windchest at index 0)
   * @param nAudioGroups the configured number of audio groups
   */
  void Resize(unsigned nWindchests, unsigned nAudioGroups);

  /**
   * @param windchestIndex the windchest to check
   * @param audioGroupId the audio group to check
   * @return true if the cell at (windchestIndex, audioGroupId) is already
   *   constructed
   */
  bool HasWindchestGroupTask(
    unsigned windchestIndex, unsigned audioGroupId) const {
    return FindWindchestGroupTask(windchestIndex, audioGroupId);
  }

  /**
   * Constructs the task at (windchestIndex, audioGroupId). Not idempotent:
   * asserts the cell was null, so every caller must first check
   * HasWindchestGroupTask() (or otherwise know the cell is unused, as
   * BuildEngine() does against a freshly-Resize()d grid).
   * @param roundCounter passed to the new task's constructor
   * @param samplerPlayer passed to the new task's constructor
   * @param windchestTask this cell's owning windchest task, passed to the
   *   new task's constructor - must be windchestIndex's own task (index 0
   *   for the detached-release row), and must already be fully constructed
   *   (callers must build every windchest task before building any grid
   *   cell, see GOSoundOrganEngine::BuildEngine()'s step ordering)
   * @param nSamplesPerBuffer passed to the new task's constructor
   * @return the newly constructed task, also owned by this grid
   */
  GOSoundWindchestGroupTask *BuildWindchestGroupTask(
    unsigned windchestIndex,
    unsigned audioGroupId,
    const GORoundCounter &roundCounter,
    GOSoundSamplerPlayer &samplerPlayer,
    GOSoundWindchestTask &windchestTask,
    unsigned nSamplesPerBuffer);

  /**
   * @return the task at (windchestIndex, audioGroupId). Asserts the cell is
   *   populated: every caller of this method is expected to only ask for a
   *   pair already known routable - either discovered by
   *   GOOrganModel::GetUsedWindchestGroupPairs() at build time, or made
   *   routable since via GOSoundOrganEngine::PrepareSoundRoutingFor() +
   *   CommitSoundRoutingFor() - so a miss indicates a bug rather than a case
   *   to handle gracefully.
   */
  GOSoundWindchestGroupTask &GetWindchestGroupTask(
    unsigned windchestIndex, unsigned audioGroupId) const;

  /**
   * @param audioGroupId the audio group to collect inputs for
   * @param nWindchests the number of windchest task slots to scan (including
   *   the detached-release windchest at index 0)
   * @return every task feeding audioGroupId, across windchestIndex
   *   [0, nWindchests), skipping unused pairs - ready to pass to
   *   GOSoundGroupTask::SetInputs()
   */
  std::vector<GOSoundBufferTaskBase *> GetInputsForGroup(
    unsigned audioGroupId, unsigned nWindchests) const;

  /** Calls fn(GOSoundWindchestGroupTask *) for every constructed task, in
   * row-major order, skipping unused cells */
  template <class Fn> void ForEachTask(Fn &&fn) const {
    for (auto &pCell : m_cells)
      if (pCell)
        fn(pCell.get());
  }

  /** Destroys every owned task and empties the grid. Defined in the .cpp
   * file because the destructor of std::unique_ptr requires the complete
   * type of the managed object, which is only forward-declared here. */
  void Clear();
};

#endif
