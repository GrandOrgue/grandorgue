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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <wx/propdlg.h>
#include <vector>

class GOrgueSound;
class SettingsAudioGroup;
class SettingsAudioOutput;
class SettingsMidiDevices;
class SettingsMidiMessage;
class SettingsOption;
class SettingsOrgan;
class SettingsReverb;

class SettingsDialog : public wxPropertySheetDialog
{
private:
	GOrgueSound& m_Sound;
	SettingsMidiDevices* m_MidiDevicePage;
	SettingsOption* m_OptionsPage;
	SettingsOrgan* m_OrganPage;
	SettingsMidiMessage* m_MidiMessagePage;
	SettingsAudioGroup* m_GroupPage;
	SettingsAudioOutput* m_OutputPage;
	SettingsReverb* m_ReverbPage;

	bool DoApply();

	void OnApply(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);

public:
	SettingsDialog(wxWindow* parent, GOrgueSound& sound);
	~SettingsDialog();

	DECLARE_EVENT_TABLE()
};


#endif
