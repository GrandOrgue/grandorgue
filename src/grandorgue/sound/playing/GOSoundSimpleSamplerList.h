/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSIMPLESAMPLERLIST_H
#define GOSOUNDSIMPLESAMPLERLIST_H

#include <atomic>

#include "GOSoundSampler.h"

class GOSoundSimpleSamplerList {
private:
  std::atomic<GOSoundSampler *> m_List;

public:
  GOSoundSimpleSamplerList() { Clear(); }

  void Clear() { m_List.store(nullptr); }

  GOSoundSampler *Get() {
    do {
      GOSoundSampler *sampler = m_List.load();
      if (!sampler)
        return NULL;
      GOSoundSampler *next = sampler->next;
      if (m_List.compare_exchange_strong(sampler, next))
        return sampler;
    } while (true);
  }

  void Put(GOSoundSampler *sampler) {
    do {
      GOSoundSampler *current = m_List.load();
      sampler->next = current;
      if (m_List.compare_exchange_strong(current, sampler))
        return;
    } while (true);
  }
};

#endif
