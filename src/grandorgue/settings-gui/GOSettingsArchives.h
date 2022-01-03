/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSETTINGSARCHIVES_H
#define GOSETTINGSARCHIVES_H

#include <wx/panel.h>

class GOConfig;
class GOSettingsOrgan;
class wxButton;
class wxListEvent;
class wxListView;

class GOSettingsArchives : public wxPanel
{
	enum {
		ID_ARCHIVES,
		ID_DEL,
	};
private:
	GOConfig& m_Settings;
	GOSettingsOrgan& m_Organs;
	wxListView* m_Archives;
	wxButton* m_Del;

	void OnArchiveSelected(wxListEvent& event);
	void OnDel(wxCommandEvent& event);

public:
	GOSettingsArchives(GOConfig& settings, GOSettingsOrgan& organs, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
