/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTHANDLERLIST_H
#define GOEVENTHANDLERLIST_H

#include <vector>

class GOCacheObject;
class GOCombinationButtonSet;
class GOControlChangedHandler;
class GOEventHandler;
class GOMidiConfigurator;
class GOSoundStateHandler;
class GOSaveableObject;

class GOEventHandlerList {
private:
  std::vector<GOCacheObject *> m_CacheObjects;
  std::vector<GOCombinationButtonSet *> m_CombinationButtonSets;
  std::vector<GOControlChangedHandler *> m_ControlChangedHandlers;
  std::vector<GOMidiConfigurator *> m_MidiConfigurators;
  std::vector<GOEventHandler *> m_MidiEventHandlers;
  std::vector<GOSoundStateHandler *> m_SoundStateHandlers;
  std::vector<GOSaveableObject *> m_SaveableObjects;

public:
  const std::vector<GOCacheObject *> &GetCacheObjects() const {
    return m_CacheObjects;
  }
  const std::vector<GOCombinationButtonSet *> &GetCombinationButtonSets()
    const {
    return m_CombinationButtonSets;
  }
  const std::vector<GOMidiConfigurator *> &GetMidiConfigurators() const {
    return m_MidiConfigurators;
  }
  const std::vector<GOEventHandler *> &GetMidiEventHandlers() const {
    return m_MidiEventHandlers;
  }
  const std::vector<GOSoundStateHandler *> &GetSoundStateHandlers() const {
    return m_SoundStateHandlers;
  }
  const std::vector<GOSaveableObject *> &GetSaveableObjects() const {
    return m_SaveableObjects;
  }

  void RegisterCacheObject(GOCacheObject *obj);
  void RegisterCombinationButtonSet(GOCombinationButtonSet *obj);
  void RegisterControlChangedHandler(GOControlChangedHandler *handler);
  void RegisterMidiConfigurator(GOMidiConfigurator *obj);
  void RegisterEventHandler(GOEventHandler *handler);
  void RegisterSoundStateHandler(GOSoundStateHandler *handler);
  void UnRegisterSoundStateHandler(GOSoundStateHandler *handler);
  void RegisterSaveableObject(GOSaveableObject *obj);
  void UnregisterSaveableObject(GOSaveableObject *obj);

  /**
   * Calls ControlChanged for all ControlChanged handlers
   * @param pControl - the control that state is changed
   */
  void SendControlChanged(void *pControl);

  void Cleanup();
};

#endif /* GOEVENTHANDLERLIST_H */
