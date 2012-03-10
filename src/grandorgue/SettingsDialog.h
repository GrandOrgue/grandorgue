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
 * MA 02111-1307, USA.
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <vector>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/propdlg.h>

class GOrgueSound;
class GOrgueSettings;

class SettingsMidiMessage;
class SettingsOption;
class SettingsOrgan;

class SettingsDialog : public wxPropertySheetDialog
{
DECLARE_CLASS(SettingsDialog)
private:
	GOrgueSound& m_Sound;
	GOrgueSettings& m_Settings;

	void SetLatencySpinner(int latency);

	wxPanel* CreateDevicesPage(wxWindow* parent);

	void UpdateSoundStatus();

	bool DoApply();

public:
	SettingsDialog(wxWindow* parent, GOrgueSound& sound);
	~SettingsDialog();

	void OnChanged(wxCommandEvent& event);
	void OnDevicesSoundChoice(wxCommandEvent& event);
	void OnDevicesMIDIClick(wxCommandEvent& event);
	void OnDevicesMIDIDoubleClick(wxCommandEvent& event);
	virtual void OnApply(wxCommandEvent& event);
	virtual void OnOK(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);

	void OnLatencySpinnerChange(wxSpinEvent& event);

	DECLARE_EVENT_TABLE()

protected:
	enum {
		ID_MIDI_DEVICES = 100,
		ID_MIDI_PROPERTIES,
		ID_SOUND_DEVICE,
		ID_LATENCY,
		ID_POLYPHONY,
		ID_MONO_STEREO,
		ID_BITS_PER_SAMPLE,
		ID_SAMPLE_RATE,
		ID_INTERPOLATION,
		ID_LOOP_LOAD,
		ID_ATTACK_LOAD,
		ID_RELEASE_LOAD,
	};

	wxCheckListBox* page1checklist;
	std::vector<int> page1checklistdata;
	wxButton* page1button;

	wxChoice* c_sound;
	wxStaticText* c_format;
	wxSpinCtrl* c_latency;
	wxStaticText* c_actual_latency;
	wxSpinCtrl* c_polyphony;
	wxChoice* c_stereo;
	wxChoice* c_SampleRate;
	wxChoice* c_BitsPerSample;
	wxChoice* c_Interpolation;
	wxChoice* c_LoopLoad;
	wxChoice* c_AttackLoad;
	wxChoice* c_ReleaseLoad;
	bool b_stereo;
	unsigned m_OldBitsPerSample;
	unsigned m_OldLoopLoad;
	unsigned m_OldAttackLoad;
	unsigned m_OldReleaseLoad;

	SettingsOption* m_OptionsPage;
	SettingsOrgan* m_OrganPage;
	SettingsMidiMessage* m_MidiMessagePage;
};


#endif
