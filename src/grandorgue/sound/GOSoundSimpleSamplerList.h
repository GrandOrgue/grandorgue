/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSIMPLESAMPLERLIST_H
#define GOSOUNDSIMPLESAMPLERLIST_H

#include "GOSoundSampler.h"
#include "threading/atomic.h"

class GOSoundSimpleSamplerList {
 private:
  atomic<GOSoundSampler*> m_List;

 public:
  GOSoundSimpleSamplerList() { Clear(); }

  void Clear() { m_List = 0; }

  GOSoundSampler* Get() {
    do {
      GOSoundSampler* sampler = m_List;
      if (!sampler) return NULL;
      GOSoundSampler* next = sampler->next;
      if (m_List.compare_exchange(sampler, next)) return sampler;
    } while (true);
  }

  void Put(GOSoundSampler* sampler) {
    do {
      GOSoundSampler* current = m_List;
      sampler->next = current;
      if (m_List.compare_exchange(current, sampler)) return;
    } while (true);
  }
};

#endif
