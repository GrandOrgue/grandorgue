/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestGroupTaskGrid.h"

#include <cassert>

#include "tasks/GOSoundWindchestGroupTask.h"

void GOSoundWindchestGroupTaskGrid::Resize(
  unsigned nWindchests, unsigned nAudioGroups) {
  m_NAudioGroups = nAudioGroups;
  m_cells.clear();
  m_cells.resize(nWindchests * nAudioGroups);
}

void GOSoundWindchestGroupTaskGrid::Clear() { m_cells.clear(); }

GOSoundWindchestGroupTask *GOSoundWindchestGroupTaskGrid::
  FindWindchestGroupTask(unsigned windchestIndex, unsigned audioGroupId) const {
  return m_cells[GetIndex(windchestIndex, audioGroupId)].get();
}

// Not idempotent: a release tail needs its own task instance at
// (0, audioGroupId), not an alias of some real windchest's task for that
// group, since it must have a separate sampler list/round from the live
// windchest's samplers (that is the entire point of detaching:
// tremulant/enclosure changes on the live windchest must not affect an
// in-flight release tail). If two different real windchests both feed the
// same audio group, cell (0, audioGroupId) must only be built once - callers
// are responsible for that via HasWindchestGroupTask(), not this method.
GOSoundWindchestGroupTask *GOSoundWindchestGroupTaskGrid::
  BuildWindchestGroupTask(
    unsigned windchestIndex,
    unsigned audioGroupId,
    const GORoundCounter &roundCounter,
    GOSoundSamplerPlayer &samplerPlayer,
    GOSoundWindchestTask &windchestTask,
    unsigned nSamplesPerBuffer) {
  std::unique_ptr<GOSoundWindchestGroupTask> &cell
    = m_cells[GetIndex(windchestIndex, audioGroupId)];

  assert(!cell);
  cell = std::make_unique<GOSoundWindchestGroupTask>(
    roundCounter, samplerPlayer, windchestTask, nSamplesPerBuffer);

  return cell.get();
}

GOSoundWindchestGroupTask &GOSoundWindchestGroupTaskGrid::GetWindchestGroupTask(
  unsigned windchestIndex, unsigned audioGroupId) const {
  GOSoundWindchestGroupTask *pTask
    = FindWindchestGroupTask(windchestIndex, audioGroupId);

  assert(pTask);

  return *pTask;
}

std::vector<GOSoundBufferTaskBase *> GOSoundWindchestGroupTaskGrid::
  GetInputsForGroup(unsigned audioGroupId, unsigned nWindchests) const {
  std::vector<GOSoundBufferTaskBase *> inputs;

  for (unsigned wcI = 0; wcI < nWindchests; wcI++) {
    GOSoundWindchestGroupTask *pTask
      = FindWindchestGroupTask(wcI, audioGroupId);

    if (pTask)
      inputs.push_back(pTask);
  }

  return inputs;
}
