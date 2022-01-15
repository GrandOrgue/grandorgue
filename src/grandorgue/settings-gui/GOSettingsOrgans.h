/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSORGANS_H
#define GOSETTINGSORGANS_H

#include <wx/panel.h>

class GOMidi;
class GOOrgan;
class GOConfig;
class wxButton;
class wxListEvent;
class wxListView;

class GOSettingsOrgans : public wxPanel {
  enum {
    ID_ORGANS = 200,
    ID_ORGAN_DEL,
    ID_ORGAN_DOWN,
    ID_ORGAN_UP,
    ID_ORGAN_TOP,
    ID_ORGAN_PROPERTIES,
    ID_PACKAGES,
    ID_PACKAGE_DEL,
  };

private:
  GOConfig &m_config;
  GOMidi &m_midi;

  wxListView *m_Organs;
  wxButton *m_OrganDown;
  wxButton *m_OrganUp;
  wxButton *m_OrganTop;
  wxButton *m_OrganDel;
  wxButton *m_OrganProperties;
  wxListView *m_Packages;
  wxButton *m_PackageDel;

  void MoveOrgan(long from, long to);

  void OnOrganSelected(wxListEvent &event);
  void OnOrganUp(wxCommandEvent &event);
  void OnOrganDown(wxCommandEvent &event);
  void OnOrganTop(wxCommandEvent &event);
  void OnOrganDel(wxCommandEvent &event);
  void OnOrganProperties(wxCommandEvent &event);

  void OnPackageSelected(wxListEvent &event);
  void OnPackageDel(wxCommandEvent &event);

public:
  GOSettingsOrgans(GOConfig &settings, GOMidi &midi, wxWindow *parent);

  void Save();

  DECLARE_EVENT_TABLE()
};

#endif
