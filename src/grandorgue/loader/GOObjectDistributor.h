/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
  std::atomic_bool m_IsBroken;

public:
  GOObjectDistributor(const std::vector<T *> &objects)
    : m_objects(objects),
      m_NObjects(objects.size()),
      m_pos(0),
      m_IsBroken(false) {}

  unsigned GetNObjects() const { return m_NObjects; }
  unsigned GetPos() const {
    unsigned pos = m_pos.load();

    return pos < m_NObjects ? pos : m_NObjects;
  }

  bool IsComplete() const { return m_pos.load() >= m_NObjects; }

  /**
   * The main method for fetching the next object. Each object can be fetched
   * only once. Returns nullptr when no unfetched objects exist or if m_IsBroken
   */
  T *FetchNext() {
    T *obj = nullptr;

    if (!m_IsBroken.load()) {
      unsigned pos = m_pos.fetch_add(1);

      if (pos < m_NObjects)
        obj = m_objects[pos];
    }
    return obj;
  }

  /**
   * This method is called on any exception occured. It causes that all worker
   * threads stop working immediate because they cannot fetch more objects
   */
  void Break() { m_IsBroken.store(true); }
};

#endif /* GOBJECTDISTRIBUTOR_H */
