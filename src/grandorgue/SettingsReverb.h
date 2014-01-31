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

#ifndef SETTINGSREVERB_H
#define SETTINGSREVERB_H

#include <wx/panel.h>

class GOrgueSettings;
class wxCheckBox;
class wxChoice;
class wxFileDirPickerEvent;
class wxFilePickerCtrl;
class wxSpinButton;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxTextCtrl;

class SettingsReverb : public wxPanel
{
	enum {
		ID_ENABLED = 200,
		ID_DIRECT,
		ID_FILE,
		ID_CHANNEL,
		ID_START_OFFSET,
		ID_LENGTH,
		ID_DELAY,
		ID_GAIN,
		ID_GAIN_SPIN,
	};
private:
	GOrgueSettings& m_Settings;
	wxCheckBox* m_Enabled;
	wxCheckBox* m_Direct;
	wxFilePickerCtrl* m_File;
	wxStaticText* m_FileName;
	wxChoice* m_Channel;
	wxSpinCtrl* m_StartOffset;
	wxSpinCtrl* m_Length;
	wxSpinCtrl* m_Delay;
	wxTextCtrl* m_Gain;
	wxSpinButton* m_GainSpin;

	void OnEnabled(wxCommandEvent& event);
	void OnFileChanged(wxFileDirPickerEvent& e);
	void OnGainSpinChanged(wxSpinEvent& e);
	void OnGainChanged(wxCommandEvent &e);

	void UpdateFile();
	void UpdateLimits();
	void UpdateEnabled();

public:
	SettingsReverb(GOrgueSettings& settings, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
