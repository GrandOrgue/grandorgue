/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTHANDLERLIST_H
#define GOEVENTHANDLERLIST_H

#include <vector>

#include "registry/GODynamicRegistry.h"
#include "registry/GOFreezableRegistry.h"

class GOCacheObject;
class GOCombinationButtonSet;
class GOControl;
class GOControlChangedHandler;
class GOEventHandler;
class GOMidiPlayingObject;
class GOReferencingObject;
class GOOrganLifecycleListener;
class GOSaveableObject;

class GOEventHandlerList {
private:
  GOFreezableRegistry<GOCacheObject> mp_CacheObjects;
  GOFreezableRegistry<GOReferencingObject> mp_ReferencingObjects;
  GOFreezableRegistry<GOCombinationButtonSet> mp_CombinationButtonSets;
  GODynamicRegistry<GOControlChangedHandler> mp_ControlChangedHandlers;
  GOFreezableRegistry<GOMidiPlayingObject> mp_MidiObjects;
  GOFreezableRegistry<GOEventHandler> mp_MidiEventHandlers;
  GOFreezableRegistry<GOOrganLifecycleListener> mp_LifecycleListeners;
  GOFreezableRegistry<GOSaveableObject> mp_SaveableObjects;

protected:
  /**
   * Freezes or unfreezes only the ReferencingObjects registry. Called by
   * GOOrganModel::Load before resolving the references, which is the only
   * reader of the list.
   */
  void SetReferencingObjectsFrozen(bool isToFreeze) {
    mp_ReferencingObjects.SetFrozen(isToFreeze);
  }

public:
  const std::vector<GOCacheObject *> &GetCacheObjects() const {
    return mp_CacheObjects.AsVector();
  }
  const std::vector<GOReferencingObject *> &GetReferencingObjects() const {
    return mp_ReferencingObjects.AsVector();
  }
  const std::vector<GOCombinationButtonSet *> &GetCombinationButtonSets()
    const {
    return mp_CombinationButtonSets.AsVector();
  }
  const std::vector<GOMidiPlayingObject *> &GetMidiObjects() const {
    return mp_MidiObjects.AsVector();
  }
  const std::vector<GOEventHandler *> &GetMidiEventHandlers() const {
    return mp_MidiEventHandlers.AsVector();
  }
  const std::vector<GOOrganLifecycleListener *> &GetLifecycleListeners() const {
    return mp_LifecycleListeners.AsVector();
  }
  const std::vector<GOSaveableObject *> &GetSaveableObjects() const {
    return mp_SaveableObjects.AsVector();
  }

  void RegisterCacheObject(GOCacheObject *obj) { mp_CacheObjects.Add(obj); }

  /** Unregisters the cache object registered by RegisterCacheObject. */
  void UnRegisterCacheObject(GOCacheObject *obj) {
    mp_CacheObjects.Remove(obj);
  }

  void RegisterReferencingObject(GOReferencingObject *pObj) {
    mp_ReferencingObjects.Add(pObj);
  }

  void UnRegisterReferencingObject(GOReferencingObject *pObj) {
    mp_ReferencingObjects.Remove(pObj);
  }

  void RegisterCombinationButtonSet(GOCombinationButtonSet *obj) {
    mp_CombinationButtonSets.Add(obj);
  }

  void UnRegisterCombinationButtonSet(GOCombinationButtonSet *obj) {
    mp_CombinationButtonSets.Remove(obj);
  }

  void RegisterControlChangedHandler(GOControlChangedHandler *handler) {
    mp_ControlChangedHandlers.Add(handler);
  }

  void UnRegisterControlChangedHandler(GOControlChangedHandler *handler) {
    mp_ControlChangedHandlers.Remove(handler);
  }

  void RegisterMidiObject(GOMidiPlayingObject *obj) { mp_MidiObjects.Add(obj); }

  void UnRegisterMidiObject(GOMidiPlayingObject *obj) {
    mp_MidiObjects.Remove(obj);
  }

  void RegisterEventHandler(GOEventHandler *handler) {
    mp_MidiEventHandlers.Add(handler);
  }

  void UnRegisterEventHandler(GOEventHandler *handler) {
    mp_MidiEventHandlers.Remove(handler);
  }

  void RegisterLifecycleListener(GOOrganLifecycleListener *handler) {
    mp_LifecycleListeners.Add(handler);
  }

  void UnRegisterLifecycleListener(GOOrganLifecycleListener *handler) {
    mp_LifecycleListeners.Remove(handler);
  }

  void RegisterSaveableObject(GOSaveableObject *obj) {
    mp_SaveableObjects.Add(obj);
  }

  void UnregisterSaveableObject(GOSaveableObject *obj) {
    mp_SaveableObjects.Remove(obj);
  }

  /**
   * Calls ControlChanged for all ControlChanged handlers
   * @param pControl - the control that state is changed
   */
  void SendControlChanged(GOControl *pControl);

  /**
   * Freezes or unfreezes all freezable registries (all but the ControlChanged
   * handlers). Idempotent.
   * @param isToFreeze false: the organ is being loaded or cleared (adding and
   *   removing allowed, reading forbidden); true: the organ is loaded (reading
   *   allowed, adding and removing forbidden)
   */
  void SetFrozen(bool isToFreeze);

  /**
   * Drops the tombstones left by the removed registrations and frees the
   * memory they occupied, keeping the registries not frozen. Called after a
   * mass removal, so that a next loading does not append after the tombstones.
   * The registries must not be frozen.
   */
  void CompactRegistries() {
    SetFrozen(true);
    SetFrozen(false);
  }
};

#endif /* GOEVENTHANDLERLIST_H */
