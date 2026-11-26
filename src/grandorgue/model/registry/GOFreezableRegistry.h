/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFREEZABLEREGISTRY_H
#define GOFREEZABLEREGISTRY_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "GORegistryBase.h"

/**
 * A non-owning registry of objects that can be frozen. Used by
 * GOEventHandlerList for the lists that are filled while an organ is loaded or
 * cleared and are only read while the organ works.
 *
 * While not frozen (constructing, loading and clearing of the organ; the
 * initial state) objects are added and removed, reading is forbidden. While
 * frozen reading is allowed, adding and removing are forbidden. A violation is
 * an assertion failure in Debug and is ignored in Release: Add and Remove do
 * nothing while frozen, AsVector returns the vector as it is while not frozen.
 *
 * The registry never deletes the registered objects.
 * @tparam T the type of the registered objects
 */
template <class T> class GOFreezableRegistry : public GORegistryBase<T> {
private:
  using GORegistryBase<T>::mp_objects;

  /** Whether the registry is frozen; not frozen initially */
  bool m_IsFrozen = false;
  /**
   * The position in mp_objects of every registered object. It exists only
   * while not frozen. The keys are non-owning pointers. It holds only the
   * live objects, so mp_objects.size() - m_IndexByObject.size() is the number
   * of the tombstones (nullptr entries left by Remove)
   */
  std::unordered_map<T *, size_t> m_IndexByObject;

  // direct access for unit tests
  friend class GOTestFreezableRegistry;

public:
  /** @return true if the registry is frozen */
  bool IsFrozen() const { return m_IsFrozen; }

  /**
   * Freezes or unfreezes the registry. Freezing compacts mp_objects, releases
   * the memory of the tombstones and frees the map; unfreezing rebuilds the
   * map. Idempotent.
   * @param isToFreeze true: freeze (reading allowed, adding and removing
   *   forbidden); false: unfreeze (the reverse)
   */
  void SetFrozen(bool isToFreeze) {
    if (isToFreeze != m_IsFrozen) {
      const size_t nObjects = mp_objects.size();

      if (isToFreeze) {
        const size_t nIndexed = m_IndexByObject.size();

        // the map holds only the live objects, mp_objects also the tombstones
        assert(nObjects >= nIndexed);
        if (nObjects > nIndexed) {
          // drop the tombstones (nullptr) keeping the order of the others
          std::erase(mp_objects, nullptr);
          mp_objects.shrink_to_fit();
        }
        // clear() would keep the buckets allocated
        std::unordered_map<T *, size_t>().swap(m_IndexByObject);
      } else {
        // fill m_IndexByObject from mp_objects: the position of every object
        size_t objectI = 0;
        auto objectIt = mp_objects.cbegin();

        m_IndexByObject.reserve(nObjects);
        for (; objectI < nObjects; ++objectI, ++objectIt)
          m_IndexByObject[*objectIt] = objectI;
      }
      m_IsFrozen = isToFreeze;
    }
  }

  /**
   * Registers the object (non-owning) if it is not registered yet. If the
   * object is already registered, nothing happens: it keeps its position and
   * is not added twice. Allowed only while not frozen; ignored in Release if
   * the registry is frozen.
   * @param pObj the object to register; must not be nullptr, because nullptr
   *   marks a tombstone
   */
  void Add(T *pObj) {
    assert(!m_IsFrozen);
    assert(pObj);
    if (!m_IsFrozen) {
      const size_t index = mp_objects.size();

      // if pObj is already registered, emplace does not change the map and
      // returns false in .second, so the vector is not changed either
      if (m_IndexByObject.emplace(pObj, index).second)
        mp_objects.push_back(pObj);
    }
  }

  /**
   * Unregisters the object if it is registered, does nothing otherwise. Does
   * not delete the object. Allowed only while not frozen; ignored in Release
   * if the registry is frozen.
   *
   * It is allowed during loading (not only during clearing) because when
   * Load() of an object throws, the destructor of the partially loaded object
   * (std::unique_ptr while the stack unwinds) calls its UnRegister... while
   * the registries are still not frozen; forbidding Remove would leave a
   * dangling pointer in the registry.
   *
   * It puts a tombstone (nullptr) in place of the object, so it takes O(1) and
   * removing ~100k pipes stays O(n). The tombstones are dropped by
   * SetFrozen(true).
   * @param pObj the object to unregister
   */
  void Remove(T *pObj) {
    assert(!m_IsFrozen);
    if (!m_IsFrozen) {
      const auto it = m_IndexByObject.find(pObj);

      if (it != m_IndexByObject.end()) {
        mp_objects[it->second] = nullptr;
        m_IndexByObject.erase(it);
      }
    }
  }

  /**
   * Hides GORegistryBase::AsVector with the check of the frozen state.
   * @return the non-owning pointers to the registered objects in the
   *   registration order. Allowed only while frozen: otherwise the vector may
   *   contain tombstones (nullptr); in Release the vector is returned as it
   *   is
   */
  const std::vector<T *> &AsVector() const {
    assert(m_IsFrozen);
    return GORegistryBase<T>::AsVector();
  }
};

#endif /* GOFREEZABLEREGISTRY_H */
