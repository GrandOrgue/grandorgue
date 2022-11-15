/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOOBJECTDISTRIBUTOR_H
#define GOOBJECTDISTRIBUTOR_H

#include <atomic>
#include <vector>

/**
 * A special object that distributes Objects across threads
 */

template <class T> class GOObjectDistributor {
private:
  const std::vector<T *> &m_objects;
  const unsigned m_NObjects;
  std::atomic_uint m_pos;

public:
  GOObjectDistributor(const std::vector<T *> &objects)
    : m_objects(objects), m_NObjects(objects.size()), m_pos(0) {}

  unsigned GetNObjects() const { return m_NObjects; }
  unsigned GetPos() const {
    unsigned pos = m_pos;

    return pos < m_NObjects ? pos : m_NObjects;
  }

  bool IsComplete() const { return m_pos >= m_NObjects; }

  /**
   * The main method for fetching the next object. Each object can be fetched
   * only once. Returns null when no unfetched objects exist
   */
  T *fetchNext() {
    unsigned pos = m_pos.fetch_add(1);

    return pos < m_NObjects ? m_objects[pos] : nullptr;
  }
};

#endif /* GOBJECTDISTRIBUTOR_H */
