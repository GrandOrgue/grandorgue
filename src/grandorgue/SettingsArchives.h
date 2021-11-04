/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSARCHIVES_H
#define SETTINGSARCHIVES_H

#include <wx/panel.h>

class GOSettings;
class SettingsOrgan;
class wxButton;
class wxListEvent;
class wxListView;

class SettingsArchives : public wxPanel
{
	enum {
		ID_ARCHIVES,
		ID_DEL,
	};
private:
	GOSettings& m_Settings;
	SettingsOrgan& m_Organs;
	wxListView* m_Archives;
	wxButton* m_Del;

	void OnArchiveSelected(wxListEvent& event);
	void OnDel(wxCommandEvent& event);

public:
	SettingsArchives(GOSettings& settings, SettingsOrgan& organs, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
