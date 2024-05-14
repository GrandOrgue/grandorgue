/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMIDIMESSAGE_H
#define GOSETTINGSMIDIMESSAGE_H

#include <wx/panel.h>

class GOMidi;
class GOConfig;
class wxButton;
class wxListEvent;
class wxListView;

class GOSettingsMidiMessage : public wxPanel {
  enum {
    ID_EVENTS,
    ID_PROPERTIES,
  };

private:
  GOConfig &m_config;
  GOMidi &m_midi;
  wxListView *m_Events;
  wxButton *m_Properties;

  void OnEventsClick(wxListEvent &event);
  void OnEventsDoubleClick(wxListEvent &event);
  void OnProperties(wxCommandEvent &event);

public:
  GOSettingsMidiMessage(GOConfig &settings, GOMidi &midi, wxWindow *parent);

  DECLARE_EVENT_TABLE()
};

#endif
