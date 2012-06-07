/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef SETTINGSMIDIDEVICES_H
#define SETTINGSMIDIDEVICES_H

#include <vector>
#include <wx/wx.h>

class GOrgueSound;

class SettingsMidiDevices : public wxPanel
{
	enum {
		ID_DEVICES = 200,
		ID_PROPERTIES,
	};
private:
	GOrgueSound& m_Sound;
	wxCheckListBox* m_Devices;
	std::vector<int> m_DeviceData;
	wxButton* m_Properties;

public:
	SettingsMidiDevices(GOrgueSound& sound, wxWindow* parent);

	void OnDevicesClick(wxCommandEvent& event);
	void OnDevicesDoubleClick(wxCommandEvent& event);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
