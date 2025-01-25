/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSBUTTONSPROXY_H
#define GOORGANSETTINGSBUTTONSPROXY_H

class GOOrganSettingsButtonsProxy {
public:
  class Listener {
  public:
    virtual void ButtonStatesChanged() = 0;
  };

private:
  Listener &r_listener;

protected:
  bool m_IsDistributeAudioEnabled = true;
  bool m_IsDefaultEnabled = true;

private:
  bool m_IsModified = false;

public:
  GOOrganSettingsButtonsProxy(Listener &listener) : r_listener(listener) {}

protected:
  void NotifyButtonStatesChanged() { r_listener.ButtonStatesChanged(); }
  bool IsModified() const { return m_IsModified; }
  void SetModified(bool newValue) { m_IsModified = newValue; }
  void NotifyModified(bool newValue = true);

public:
  bool IsDistributeAudioEnabled() const { return m_IsDistributeAudioEnabled; }
  bool IsDefaultEnabled() const { return m_IsDefaultEnabled; }
  bool IsRevertEnabled() const { return IsModified(); }
  bool IsApplyEnabled() const { return IsModified(); }
};

#endif /* GOORGANSETTINGSBUTTONSPROXY_H */
