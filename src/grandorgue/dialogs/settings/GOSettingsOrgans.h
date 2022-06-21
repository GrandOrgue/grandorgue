/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSORGANS_H
#define GOSETTINGSORGANS_H

#include <wx/panel.h>

#include "ptrvector.h"

class GOMidi;
class wxButton;
class wxListEvent;
class wxListView;
class wxTextCtrl;

class GOArchiveFile;
class GOConfig;
class GOOrgan;

class GOSettingsOrgans : public wxPanel {
  enum {
    ID_ORGANS = 200,
    ID_ORGAN_DEL,
    ID_ORGAN_DOWN,
    ID_ORGAN_UP,
    ID_ORGAN_TOP,
    ID_ORGAN_PROPERTIES
  };

private:
  struct OrganRec {
    const GOOrgan *p_organ;
    bool is_selected;
    bool is_focused;
  };
  using OrganRecs = std::vector<OrganRec>;

  GOConfig &m_config;
  GOMidi &m_midi;
  ptr_vector<GOOrgan> &m_OrigOrganList;
  std::vector<GOArchiveFile *> m_PackageList;

  wxListView *m_Organs;
  wxTextCtrl *m_Builder;
  wxTextCtrl *m_Recording;
  wxTextCtrl *m_OrganHash;
  wxTextCtrl *m_PackageId;
  wxTextCtrl *m_PackageName;
  wxTextCtrl *m_PathInPackage;
  wxTextCtrl *m_PackageHash;
  wxTextCtrl *m_PackageInfo;

  wxButton *m_OrganDown;
  wxButton *m_OrganUp;
  wxButton *m_OrganTop;
  wxButton *m_OrganDel;
  wxButton *m_OrganProperties;

  OrganRecs GetCurrentOrganRecs();
  void ReorderOrgans(const OrganRecs &newSortedRecs);
  void MoveOrgan(long from, long to);

  void OnOrganFocused(wxListEvent &event);
  void OnOrganSelected(wxListEvent &event);
  void OnOrganUp(wxCommandEvent &event);
  void OnOrganDown(wxCommandEvent &event);
  void OnOrganTop(wxCommandEvent &event);
  void OnOrganDel(wxCommandEvent &event);
  void OnOrganProperties(wxCommandEvent &event);

public:
  GOSettingsOrgans(GOConfig &settings, GOMidi &midi, wxWindow *parent);

  virtual bool TransferDataToWindow() override;
  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
