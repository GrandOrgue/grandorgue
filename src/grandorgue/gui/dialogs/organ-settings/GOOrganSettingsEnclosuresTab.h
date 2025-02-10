/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
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

#include "GOOrganSettingsButtonsProxy.h"

class wxListBox;
class wxTextCtrl;
class wxTreeCtrl;
class wxTreeEvent;

class GOEnclosure;
class GOOrganModel;
class GOWindchest;

class GOOrganSettingsEnclosuresTab : public wxPanel,
                                     public GOOrganSettingsButtonsProxy {
private:
  class ItemData;

  GOOrganModel &r_OrganModel;

  wxTreeCtrl *m_tree;
  wxListBox *m_WindchestList;
  wxTextCtrl *m_MinAmpLevelEdit;

  std::unordered_map<GOEnclosure *, std::vector<wxString>>
    m_WindchestsByEnclosures;

public:
  GOOrganSettingsEnclosuresTab(
    GOOrganModel &organModel,
    wxWindow *parent,
    GOOrganSettingsButtonsProxy::Listener &listener);

private:
  void LoadValues();
  bool TransferDataToWindow() override;
  void OnTreeChanging(wxTreeEvent &e);
  void OnTreeChanged(wxTreeEvent &e) { LoadValues(); }
  void OnMinAmpLevelChanged(wxCommandEvent &e) { NotifyModified(); }
  void DoForAllEnclosures(const std::function<void(GOEnclosure &enclosure)> &f);

public:
  void ResetToDefault();
  void DiscardChanges() { LoadValues(); }
  void ApplyChanges();

  bool Validate() override { return !CheckForUnapplied(); }

  DECLARE_EVENT_TABLE()
};

#endif /* GOORGANSETTINGSENCLOSURESTAB_H */
