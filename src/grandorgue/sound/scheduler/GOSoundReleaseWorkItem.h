/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRELEASEWORKITEM_H
#define GOSOUNDRELEASEWORKITEM_H

#include "ptrvector.h"
#include "sound/GOSoundSimpleSamplerList.h"
#include "sound/scheduler/GOSoundWorkItem.h"

class GOSoundEngine;
class GOSoundGroupWorkItem;
class GOSoundSampler;

class GOSoundReleaseWorkItem : public GOSoundWorkItem {
 private:
  GOSoundEngine &m_engine;
  ptr_vector<GOSoundGroupWorkItem> &m_AudioGroups;
  GOSoundSimpleSamplerList m_List;
  atomic_uint m_WaitCnt;
  atomic_uint m_Cnt;
  volatile bool m_Stop;

 public:
  GOSoundReleaseWorkItem(
    GOSoundEngine &sound_engine,
    ptr_vector<GOSoundGroupWorkItem> &audio_groups);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();

  void Add(GOSoundSampler *sampler);
};

#endif
