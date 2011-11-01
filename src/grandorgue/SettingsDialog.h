/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
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
class wxConfigBase;
class wxListEvent;
class wxListView;
class wxPanel;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxToggleButton;

class GOrgueSettings;

class SettingsDialog : public wxPropertySheetDialog
{
DECLARE_CLASS(SettingsDialog)
private:
	GOrgueSettings& m_Settings;

	void SetLatencySpinner(int latency);

public:
	SettingsDialog(wxWindow* parent, GOrgueSettings& settings);
	~SettingsDialog();

	void UpdateSoundStatus();
	wxPanel* CreateDevicesPage(wxWindow* parent);
	wxPanel* CreateMessagesPage(wxWindow* parent);
	wxPanel* CreateOrganPage(wxWindow* parent);
	void UpdateMessages(int i);
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
		ID_ENHANCE_SQUASH,
		ID_ENHANCE_MANAGE_POLYPHONY,
		ID_COMPRESS_CACHE,
		ID_ENHANCE_SCALE_RELEASE,
		ID_ENHANCE_RANDOMIZE,

		ID_MIDI_EVENTS,
		ID_EVENT_PROPERTIES,
		ID_ORGAN_EVENTS,
		ID_EVENT_DEL,
		ID_EVENT_ADD,
		ID_EVENT_ORGANPROPERTIES
	};

	wxConfigBase *pConfig;

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
	wxChoice* c_Concurrency;
	wxChoice* c_ReleaseConcurrency;
	wxChoice* c_WaveFormat;
	wxCheckBox *c_squash, *c_limit, *c_CompressCache, *c_scale, *c_random;
	bool b_squash, b_stereo;

	wxListView* page2list, *organlist;
	wxButton* page2button, *addButton, *delButton, *propButton;
};


#endif
