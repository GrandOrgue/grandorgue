/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SETTINGSTEMPERAMENTS_H
#define SETTINGSTEMERPAMENTS_H

#include <vector>
#include <wx/panel.h>

class GOrgueSettings;
class GOrgueTemperamentList;
class GOrgueTemperamentUser;
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
	GOrgueTemperamentList& m_Temperaments;
	std::vector<GOrgueTemperamentUser*> m_Ptrs;
	wxGrid* m_List;
	wxButton* m_Add;
	wxButton* m_Del;

	void OnListSelected(wxGridEvent& event);
	void OnAdd(wxCommandEvent& event);
	void OnDel(wxCommandEvent& event);

	void Update();

public:
	SettingsTemperaments(GOrgueSettings& settings, wxWindow* parent);

	void Save();
	
	DECLARE_EVENT_TABLE()
};

#endif
