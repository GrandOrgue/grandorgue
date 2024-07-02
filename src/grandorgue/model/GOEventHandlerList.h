/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTHANDLERLIST_H
#define GOEVENTHANDLERLIST_H

#include <algorithm>
#include <unordered_set>
#include <vector>

class GOCacheObject;
class GOCombinationButtonSet;
class GOControl;
class GOControlChangedHandler;
class GOEventHandler;
class GOMidiConfigurator;
class GOSoundStateHandler;
class GOSaveableObject;

class GOEventHandlerList {
private:
  template <class T> class UPVector {
  private:
    std::vector<T *> objVector;
    std::unordered_set<T *> objSet;

  public:
    void Add(T *pObj) {
      if (objSet.find(pObj) == objSet.end()) {
        objSet.insert(pObj);
        objVector.push_back(pObj);
      }
    }

    void Remove(T *pObj) {
      auto sIt = objSet.find(pObj);

      if (sIt != objSet.end()) {
        auto vIt = std::find(objVector.begin(), objVector.end(), pObj);

        if (vIt != objVector.end())
          objVector.erase(vIt);
        objSet.erase(sIt);
      }
    }

    const std::vector<T *> &AsVector() const { return objVector; }

    void Clear() {
      objVector.clear();
      objSet.clear();
    }
  };

  UPVector<GOCacheObject> m_CacheObjects;
  UPVector<GOCombinationButtonSet> m_CombinationButtonSets;
  UPVector<GOControlChangedHandler> m_ControlChangedHandlers;
  UPVector<GOMidiConfigurator> m_MidiConfigurators;
  UPVector<GOEventHandler> m_MidiEventHandlers;
  UPVector<GOSoundStateHandler> m_SoundStateHandlers;
  UPVector<GOSaveableObject> m_SaveableObjects;

public:
  const std::vector<GOCacheObject *> &GetCacheObjects() const {
    return m_CacheObjects.AsVector();
  }
  const std::vector<GOCombinationButtonSet *> &GetCombinationButtonSets()
    const {
    return m_CombinationButtonSets.AsVector();
  }
  const std::vector<GOMidiConfigurator *> &GetMidiConfigurators() const {
    return m_MidiConfigurators.AsVector();
  }
  const std::vector<GOEventHandler *> &GetMidiEventHandlers() const {
    return m_MidiEventHandlers.AsVector();
  }
  const std::vector<GOSoundStateHandler *> &GetSoundStateHandlers() const {
    return m_SoundStateHandlers.AsVector();
  }
  const std::vector<GOSaveableObject *> &GetSaveableObjects() const {
    return m_SaveableObjects.AsVector();
  }

  void RegisterCacheObject(GOCacheObject *obj) { m_CacheObjects.Add(obj); }

  void RegisterCombinationButtonSet(GOCombinationButtonSet *obj) {
    m_CombinationButtonSets.Add(obj);
  }

  void RegisterControlChangedHandler(GOControlChangedHandler *handler) {
    m_ControlChangedHandlers.Add(handler);
  }

  void UnRegisterControlChangedHandler(GOControlChangedHandler *handler) {
    m_ControlChangedHandlers.Remove(handler);
  }

  void RegisterMidiConfigurator(GOMidiConfigurator *obj) {
    m_MidiConfigurators.Add(obj);
  }

  void RegisterEventHandler(GOEventHandler *handler) {
    m_MidiEventHandlers.Add(handler);
  }

  void RegisterSoundStateHandler(GOSoundStateHandler *handler) {
    m_SoundStateHandlers.Add(handler);
  }

  void UnRegisterSoundStateHandler(GOSoundStateHandler *handler) {
    m_SoundStateHandlers.Remove(handler);
  }

  void RegisterSaveableObject(GOSaveableObject *obj) {
    m_SaveableObjects.Add(obj);
  }

  void UnregisterSaveableObject(GOSaveableObject *obj) {
    m_SaveableObjects.Remove(obj);
  }

  /**
   * Calls ControlChanged for all ControlChanged handlers
   * @param pControl - the control that state is changed
   */
  void SendControlChanged(GOControl *pControl);

  void Cleanup();
};

#endif /* GOEVENTHANDLERLIST_H */
