/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSMIDIMESSAGE_H
#define SETTINGSMIDIMESSAGE_H

#include <wx/panel.h>

class GOMidi;
class GOSettings;
class wxButton;
class wxListEvent;
class wxListView;

class SettingsMidiMessage : public wxPanel
{
	enum {
		ID_EVENTS,
		ID_PROPERTIES,
	};

private:
	GOSettings& m_Settings;
	GOMidi& m_midi;
	wxListView* m_Events;
	wxButton* m_Properties;

	void OnEventsClick(wxListEvent& event);
	void OnEventsDoubleClick(wxListEvent& event);
	void OnProperties(wxCommandEvent& event);

public:
	SettingsMidiMessage(GOSettings& settings, GOMidi& midi, wxWindow* parent);

	DECLARE_EVENT_TABLE()
};

#endif

