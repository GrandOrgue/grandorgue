/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTHANDLERLIST_H
#define GOEVENTHANDLERLIST_H

#include <vector>

class GOCacheObject;
class GOControlChangedHandler;
class GOEventHandler;
class GOMidiConfigurator;
class GOPlaybackStateHandler;
class GOSaveableObject;

class GOEventHandlerList {
private:
  std::vector<GOCacheObject *> m_CacheObjects;
  std::vector<GOControlChangedHandler *> m_ControlChangedHandlers;
  std::vector<GOMidiConfigurator *> m_MidiConfigurators;
  std::vector<GOEventHandler *> m_MidiEventHandlers;
  std::vector<GOPlaybackStateHandler *> m_PlaybackStateHandlers;
  std::vector<GOSaveableObject *> m_SaveableObjects;

public:
  const std::vector<GOCacheObject *> &GetCacheObjects() const {
    return m_CacheObjects;
  }
  const std::vector<GOControlChangedHandler *> &GetControlChangedHandlers()
    const {
    return m_ControlChangedHandlers;
  }
  const std::vector<GOMidiConfigurator *> &GetMidiConfigurators() const {
    return m_MidiConfigurators;
  }
  const std::vector<GOEventHandler *> &GetMidiEventHandlers() const {
    return m_MidiEventHandlers;
  }
  const std::vector<GOPlaybackStateHandler *> &GetPlaybackStateHandlers()
    const {
    return m_PlaybackStateHandlers;
  }
  const std::vector<GOSaveableObject *> &GetSaveableObjects() const {
    return m_SaveableObjects;
  }

  void RegisterCacheObject(GOCacheObject *obj);
  void RegisterControlChangedHandler(GOControlChangedHandler *handler);
  void RegisterMidiConfigurator(GOMidiConfigurator *obj);
  void RegisterEventHandler(GOEventHandler *handler);
  void RegisterPlaybackStateHandler(GOPlaybackStateHandler *handler);
  void RegisterSaveableObject(GOSaveableObject *obj);
  void UnregisterSaveableObject(GOSaveableObject *obj);

  void Cleanup();
};

#endif /* GOEVENTHANDLERLIST_H */
