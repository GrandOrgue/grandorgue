/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSTEMPERAMENTS_H
#define GOSETTINGSTEMPERAMENTS_H

#include <wx/panel.h>

#include <vector>

class GOConfig;
class GOTemperamentList;
class GOTemperamentUser;
class wxButton;
class wxGrid;
class wxGridEvent;

class GOSettingsTemperaments : public wxPanel {
  enum {
    ID_LIST,
    ID_ADD,
    ID_DEL,
  };

private:
  GOTemperamentList &m_Temperaments;
  std::vector<GOTemperamentUser *> m_Ptrs;
  wxGrid *m_List;
  wxButton *m_Add;
  wxButton *m_Del;

  void OnListSelected(wxGridEvent &event);
  void OnAdd(wxCommandEvent &event);
  void OnDel(wxCommandEvent &event);

  void Update();

public:
  GOSettingsTemperaments(GOConfig &settings, wxWindow *parent);

  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
