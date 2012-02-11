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
#include <wx/propdlg.h>

class wxButton;
class wxCheckBox;
class wxCheckListBox;
class wxChoice;
class wxDirPickerCtrl;
class wxListEvent;
class wxListView;
class wxPanel;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxToggleButton;

class GOrgueSound;
class GOrgueSettings;

#include "MIDIListenDialog.h"

class SettingsDialog : public wxPropertySheetDialog
{
DECLARE_CLASS(SettingsDialog)
private:
	GOrgueSound& m_Sound;
	GOrgueSettings& m_Settings;

	void SetLatencySpinner(int latency);
	MIDIListenDialog::LISTEN_DIALOG_TYPE GetEventType(unsigned index);
	unsigned GetEventCount();
	wxString GetEventName(unsigned index);
	int GetEventData(unsigned index);
	void SetEventData(unsigned index, int event);

public:
	SettingsDialog(wxWindow* parent, GOrgueSound& sound);
	~SettingsDialog();

	void UpdateSoundStatus();
	wxPanel* CreateDevicesPage(wxWindow* parent);
	wxPanel* CreateOptionsPage(wxWindow* parent);
	wxPanel* CreateMessagesPage(wxWindow* parent);
	wxPanel* CreateOrganPage(wxWindow* parent);
	void UpdateMessages(unsigned i);
	void UpdateOrganMessages(int i);

	void OnChanged(wxCommandEvent& event);
	void OnDevicesSoundChoice(wxCommandEvent& event);
	void OnDevicesMIDIClick(wxCommandEvent& event);
	void OnDevicesMIDIDoubleClick(wxCommandEvent& event);
	virtual void OnApply(wxCommandEvent& event);
	virtual void OnOK(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
	bool DoApply();

	void OnEventListClick(wxListEvent& event);
	void OnEventListDoubleClick(wxListEvent& event);
	void OnEventListDoubleClickC(wxCommandEvent& event);

	void OnOrganEventListClick(wxListEvent& event);
	void OnOrganEventListDoubleClick(wxListEvent& event);
	void OnAddOrgan(wxCommandEvent& event);
	void OnDelOrgan(wxCommandEvent& event);
	void OnOrganProperties(wxCommandEvent& event);

	void OnLatencySpinnerChange(wxSpinEvent& event);

	EVENT_HANDLER_REALIGN;
	DECLARE_EVENT_TABLE()

protected:
	enum {
		ID_MIDI_DEVICES = 100,
		ID_MIDI_PROPERTIES,
		ID_SOUND_DEVICE,
		ID_LATENCY,
		ID_POLYPHONY,
		ID_MONO_STEREO,
		ID_SAMPLE_RATE,
		ID_CONCURRENCY,
		ID_RELEASE_CONCURRENCY,
		ID_WAVE_FORMAT,
		ID_INTERPOLATION,
		ID_LOOP_LOAD,
		ID_ENHANCE_SQUASH,
		ID_ENHANCE_MANAGE_POLYPHONY,
		ID_COMPRESS_CACHE,
		ID_ENHANCE_SCALE_RELEASE,
		ID_ENHANCE_RANDOMIZE,
		ID_SETTINGS_DIR,
		ID_CACHE_DIR,

		ID_MIDI_EVENTS,
		ID_EVENT_PROPERTIES,
		ID_ORGAN_EVENTS,
		ID_EVENT_DEL,
		ID_EVENT_ADD,
		ID_EVENT_ORGANPROPERTIES
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
	wxChoice* c_Concurrency;
	wxChoice* c_ReleaseConcurrency;
	wxChoice* c_WaveFormat;
	wxChoice* c_Interpolation;
	wxChoice* c_LoopLoad;
	wxCheckBox *c_squash, *c_limit, *c_CompressCache, *c_scale, *c_random;
	wxDirPickerCtrl* c_SettingsPath;
	wxDirPickerCtrl* c_CachePath;
	bool b_squash, b_stereo;
	unsigned m_OldBitsPerSample;
	unsigned m_OldLoopLoad;

	wxListView* page2list, *organlist;
	wxButton* page2button, *addButton, *delButton, *propButton;
};


#endif
