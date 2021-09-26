/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSMIDIMESSAGE_H
#define SETTINGSMIDIMESSAGE_H

#include <wx/panel.h>

class GOrgueMidi;
class GOrgueSettings;
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
	GOrgueSettings& m_Settings;
	GOrgueMidi& m_midi;
	wxListView* m_Events;
	wxButton* m_Properties;

	void OnEventsClick(wxListEvent& event);
	void OnEventsDoubleClick(wxListEvent& event);
	void OnProperties(wxCommandEvent& event);

public:
	SettingsMidiMessage(GOrgueSettings& settings, GOrgueMidi& midi, wxWindow* parent);

	DECLARE_EVENT_TABLE()
};

#endif

