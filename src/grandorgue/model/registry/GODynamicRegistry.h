/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODYNAMICREGISTRY_H
#define GODYNAMICREGISTRY_H

#include <algorithm>

#include "GORegistryBase.h"

/**
 * A non-owning registry without freezing: Add, Remove and reading are always
 * allowed. Intended for small lists of objects that are registered and
 * unregistered at runtime (for example GOStopsWindow as a
 * ControlChangedHandler): Add and Remove take O(n). Remove keeps the order.
 * The registered objects are never deleted by the registry. Used by
 * GOEventHandlerList.
 * @tparam T the type of the registered objects
 */
template <class T> class GODynamicRegistry : public GORegistryBase<T> {
private:
  using GORegistryBase<T>::mp_objects;

public:
  /** Registers the object (non-owning) if it is not registered yet. */
  void Add(T *pObj) {
    if (
      std::find(mp_objects.begin(), mp_objects.end(), pObj) == mp_objects.end())
      mp_objects.push_back(pObj);
  }

  /** Unregisters the object if it is registered. Does not delete it. */
  void Remove(T *pObj) {
    const auto it = std::find(mp_objects.begin(), mp_objects.end(), pObj);

    if (it != mp_objects.end())
      mp_objects.erase(it);
  }
};

#endif /* GODYNAMICREGISTRY_H */
