/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREGISTRYBASE_H
#define GOREGISTRYBASE_H

#include <vector>

/**
 * Base class of the registries of objects used by GOEventHandlerList. Keeps
 * the registered objects in the registration order. The registration is
 * non-owning: a registry never deletes the registered objects, they must be
 * removed from the registry before they are destroyed. Add and Remove are
 * defined by the subclasses. There are no virtual methods: the registries are
 * never used polymorphically and AsVector is called for every MIDI event.
 * @tparam T the type of the registered objects
 */
template <class T> class GORegistryBase {
protected:
  /** The non-owning pointers to the registered objects in the registration
   * order */
  std::vector<T *> mp_objects;

  // not for deleting through the base class
  ~GORegistryBase() = default;

public:
  /** @return the non-owning pointers to the registered objects in the
   * registration order. A subclass may hide it with additional checks */
  const std::vector<T *> &AsVector() const { return mp_objects; }
};

#endif /* GOREGISTRYBASE_H */
