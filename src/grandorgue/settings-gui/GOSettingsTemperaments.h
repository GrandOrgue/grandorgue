/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSTEMPERAMENTS_H
#define SETTINGSTEMPERAMENTS_H

#include <vector>
#include <wx/panel.h>

class GOConfig;
class GOTemperamentList;
class GOTemperamentUser;
class wxButton;
class wxGrid;
class wxGridEvent;

class SettingsTemperaments : public wxPanel
{
	enum {
		ID_LIST,
		ID_ADD,
		ID_DEL,
	};
private:
	GOTemperamentList& m_Temperaments;
	std::vector<GOTemperamentUser*> m_Ptrs;
	wxGrid* m_List;
	wxButton* m_Add;
	wxButton* m_Del;

	void OnListSelected(wxGridEvent& event);
	void OnAdd(wxCommandEvent& event);
	void OnDel(wxCommandEvent& event);

	void Update();

public:
	SettingsTemperaments(GOConfig& settings, wxWindow* parent);

	void Save();
	
	DECLARE_EVENT_TABLE()
};

#endif
