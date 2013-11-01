/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef SETTINGSORGAN_H
#define SETTINGSORGAN_H

#include <wx/wx.h>
#include <wx/listctrl.h>

class GOrgueMidi;
class GOrgueSettings;

class SettingsOrgan : public wxPanel
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
	GOrgueSettings& m_Settings;
	GOrgueMidi& m_midi;
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
	SettingsOrgan(GOrgueSettings& settings, GOrgueMidi& midi, wxWindow* parent);

	void Save();
	
	DECLARE_EVENT_TABLE()
};

#endif
