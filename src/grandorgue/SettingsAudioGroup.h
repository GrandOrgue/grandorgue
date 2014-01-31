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

#ifndef SETTINGSAUDIOGROUP_H
#define SETTINGSAUDIOGROUP_H

#include <wx/panel.h>
#include <vector>

class GOrgueSettings;
class wxButton;
class wxListBox;

class GOAudioGroupCallback
{
public:
	virtual std::vector<wxString> GetGroups() = 0;
};

class SettingsAudioGroup : public wxPanel, public GOAudioGroupCallback
{
	enum {
		ID_AUDIOGROUP_LIST = 200,
		ID_AUDIOGROUP_ADD,
		ID_AUDIOGROUP_DEL,
		ID_AUDIOGROUP_CHANGE,
	};

private:
	GOrgueSettings& m_Settings;
	wxListBox* m_AudioGroups;
	wxButton* m_Add;
	wxButton* m_Del;
	wxButton* m_Change;

	void OnGroup(wxCommandEvent& event);
	void OnGroupAdd(wxCommandEvent& event);
	void OnGroupDel(wxCommandEvent& event);
	void OnGroupChange(wxCommandEvent& event);

public:
	SettingsAudioGroup(GOrgueSettings& settings, wxWindow* parent);

	std::vector<wxString> GetGroups();

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
