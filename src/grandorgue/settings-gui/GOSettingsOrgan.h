/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSETTINGSORGAN_H
#define GOSETTINGSORGAN_H

#include <wx/panel.h>
#include <vector>

class GOMidi;
class GOOrgan;
class GOConfig;
class wxButton;
class wxListEvent;
class wxListView;

class GOSettingsOrgan : public wxPanel
{
	enum {
		ID_ORGANS,
		ID_DEL,
		ID_UP,
		ID_DOWN,
		ID_TOP,
		ID_PROPERTIES,
	};
private:
	GOConfig& m_Settings;
	GOMidi& m_midi;
	wxListView* m_Organs;
	wxButton* m_Up;
	wxButton* m_Down;
	wxButton* m_Top;
	wxButton* m_Del;
	wxButton* m_Properties;

	void OnOrganSelected(wxListEvent& event);
	void OnUp(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnTop(wxCommandEvent& event);
	void OnDel(wxCommandEvent& event);
	void OnProperties(wxCommandEvent& event);

	void MoveOrgan(long from, long to);

public:
	GOSettingsOrgan(GOConfig& settings, GOMidi& midi, wxWindow* parent);

	std::vector<const GOOrgan*> GetOrgans();

	void Save();
	
	DECLARE_EVENT_TABLE()
};

#endif
