/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLERLIST_H
#define GOSOUNDSAMPLERLIST_H

#include <atomic>

#include "GOSoundSampler.h"

class GOSoundSamplerList {
private:
  std::atomic<GOSoundSampler *> m_GetList;
  std::atomic<GOSoundSampler *> m_PutList;
  std::atomic_uint m_PutCount;

public:
  GOSoundSamplerList() { Clear(); }

  void Clear() {
    m_GetList.store(0);
    m_PutList.store(0);
    m_PutCount.store(0);
  }

  GOSoundSampler *Peek() { return m_GetList.load(); }

  GOSoundSampler *Get() {
    do {
      GOSoundSampler *sampler = m_GetList.load();
      if (!sampler)
        return NULL;
      GOSoundSampler *next = sampler->next;
      if (m_GetList.compare_exchange_strong(sampler, next))
        return sampler;
    } while (true);
  }

  void Put(GOSoundSampler *sampler) {
    do {
      GOSoundSampler *current = m_PutList;
      sampler->next = current;
      if (m_PutList.compare_exchange_strong(current, sampler)) {
        m_PutCount.fetch_add(1);
        return;
      }
    } while (true);
  }

  unsigned GetCount() { return m_PutCount; }

  void Move() {
    GOSoundSampler *sampler;
    do {
      sampler = m_PutList;
      if (m_PutList.compare_exchange_strong(sampler, nullptr))
        break;
    } while (true);
    m_PutCount.exchange(0);

    if (!sampler)
      return;
    do {
      GOSoundSampler *current = m_GetList.load();
      GOSoundSampler *next = sampler;
      if (current) {
        while (next) {
          if (next->next)
            next = next->next;
          else {
            next->next = current;
            break;
          }
        }
      }
      if (m_GetList.compare_exchange_strong(current, sampler))
        return;
      if (next)
        next->next = NULL;
    } while (true);
  }
};

#endif
