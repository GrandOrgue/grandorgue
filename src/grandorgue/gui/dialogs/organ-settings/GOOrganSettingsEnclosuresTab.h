/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSENCLOSURESTAB_H
#define GOORGANSETTINGSENCLOSURESTAB_H

#include <functional>
#include <unordered_map>
#include <vector>

#include <wx/event.h>
#include <wx/panel.h>

#include "GOOrganSettingsTab.h"

class wxCheckBox;
class wxListBox;
class wxTextCtrl;
class wxTreeCtrl;
class wxTreeEvent;

class GOEnclosure;
class GOOrganModel;
class GOWindchest;

class GOOrganSettingsEnclosuresTab : public GOOrganSettingsTab {
private:
  class ItemData;

  GOOrganModel &r_OrganModel;

  wxTreeCtrl *m_tree;
  wxCheckBox *m_IsOdfDefined;
  wxListBox *m_WindchestList;
  wxTextCtrl *m_MinAmpLevelEdit;
  wxTextCtrl *m_LowShelfFrequencyEdit;
  wxTextCtrl *m_LowShelfAttenuationDbEdit;
  wxTextCtrl *m_HighShelfFrequencyEdit;
  wxTextCtrl *m_HighShelfAttenuationDbEdit;

  std::unordered_map<GOEnclosure *, wxArrayString> m_WindchestsByEnclosures;

public:
  GOOrganSettingsEnclosuresTab(
    GOOrganModel &organModel, GOOrganSettingsDialogBase *pDlg);

private:
  void LoadValues();
  bool TransferDataToWindow() override;
  void OnTreeChanging(wxTreeEvent &e);
  void OnTreeChanged(wxTreeEvent &e) { LoadValues(); }
  void OnMinAmpLevelChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnLowShelfFrequencyChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnLowShelfAttenuationDbChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnHighShelfFrequencyChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnHighShelfAttenuationDbChanged(wxCommandEvent &e) { NotifyModified(); }
  void DoForAllEnclosures(const std::function<void(GOEnclosure &enclosure)> &f);

  /**
   * Applies the modified shelf edits to all selected enclosures. Nothing is
   * applied (and an error is shown) if any edited value is out of its range
   * or if a shelf of any selected enclosure would get an attenuation greater
   * than 0 without a frequency greater than 0.
   */
  void ApplyShelfChanges();

public:
  void ResetToDefault() override;
  void DiscardChanges() override { LoadValues(); }
  void ApplyChanges() override;

  DECLARE_EVENT_TABLE()
};

#endif /* GOORGANSETTINGSENCLOSURESTAB_H */
