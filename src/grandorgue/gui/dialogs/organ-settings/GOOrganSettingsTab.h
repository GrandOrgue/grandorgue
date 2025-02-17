/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSBUTTONSPROXY_H
#define GOORGANSETTINGSBUTTONSPROXY_H

#include "gui/dialogs/common/GODialogTab.h"

class GOOrganSettingsDialogBase;

class GOOrganSettingsTab : public GODialogTab {
public:
private:
  GOOrganSettingsDialogBase *p_dlg;

protected:
  bool m_IsDistributeAudioEnabled = false;
  bool m_IsDefaultEnabled = true;

private:
  bool m_IsModified = false;

public:
  GOOrganSettingsTab(
    GOOrganSettingsDialogBase *pDlg,
    const wxString &name,
    const wxString &label);

protected:
  void NotifyButtonStatesChanged();
  bool IsModified() const { return m_IsModified; }
  void SetModified(bool newValue) { m_IsModified = newValue; }
  void NotifyModified(bool newValue = true);

public:
  bool IsDistributeAudioEnabled() const {
    return m_IsDistributeAudioEnabled && !IsModified();
  }

  bool IsDefaultEnabled() const { return m_IsDefaultEnabled && !IsModified(); }

  bool IsRevertEnabled() const { return IsModified(); }
  bool IsApplyEnabled() const { return IsModified(); }

  virtual void DistributeAudio() {}
  virtual void ResetToDefault() = 0;
  virtual void DiscardChanges() = 0;
  virtual void ApplyChanges() = 0;

  /**
   * Checks if all changes have been applied. If some unapplied changes are
   * present, then display an error message.
   * Returns if there are unapplied changes
   */
  bool CheckForUnapplied();
  bool Validate() override { return !CheckForUnapplied(); }
};

#endif /* GOORGANSETTINGSBUTTONSPROXY_H */
