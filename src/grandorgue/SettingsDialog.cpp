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

#include <wx/spinctrl.h>
#include <wx/bookctrl.h>
#include <wx/numdlg.h>

#include "SettingsDialog.h"
#include "GrandOrgueID.h"
#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GOrgueRtHelpers.h"
#include "SettingsMidiMessage.h"
#include "SettingsOption.h"
#include "SettingsOrgan.h"

IMPLEMENT_CLASS(SettingsDialog, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(SettingsDialog, wxPropertySheetDialog)
	EVT_CHECKLISTBOX(ID_MIDI_DEVICES, SettingsDialog::OnChanged)
	EVT_LISTBOX(ID_MIDI_DEVICES, SettingsDialog::OnDevicesMIDIClick)
	EVT_LISTBOX_DCLICK(ID_MIDI_DEVICES, SettingsDialog::OnDevicesMIDIDoubleClick)
	EVT_BUTTON(ID_MIDI_PROPERTIES, SettingsDialog::OnDevicesMIDIDoubleClick)
	EVT_CHOICE(ID_SOUND_DEVICE, SettingsDialog::OnDevicesSoundChoice)
	EVT_CHOICE(ID_MONO_STEREO, SettingsDialog::OnChanged)
	EVT_CHOICE(ID_SAMPLE_RATE, SettingsDialog::OnChanged)
	EVT_CHOICE(ID_LOOP_LOAD, SettingsDialog::OnChanged)
	EVT_CHOICE(ID_ATTACK_LOAD, SettingsDialog::OnChanged)
	EVT_CHOICE(ID_RELEASE_LOAD, SettingsDialog::OnChanged)
	EVT_CHOICE(ID_INTERPOLATION, SettingsDialog::OnChanged)

	EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
	EVT_BUTTON(wxID_HELP, SettingsDialog::OnHelp)

	EVT_SPINCTRL(ID_LATENCY, SettingsDialog::OnLatencySpinnerChange)

END_EVENT_TABLE()

void SettingsDialog::SetLatencySpinner(int latency)
{

	int corresponding_estimated_latency = m_Settings.GetAudioDeviceLatency(c_sound->GetStringSelection());

	if (c_latency->GetValue() != latency)
		c_latency->SetValue(latency);

	wxString lat_s = wxT("---");
	if (latency == corresponding_estimated_latency)
	{

		long int lat_l = m_Settings.GetAudioDeviceActualLatency(c_sound->GetStringSelection());
		if (lat_l > 0)
			lat_s.Printf(_("%ld ms"), lat_l);

	}

	c_actual_latency->SetLabel(lat_s);

}

#define SETTINGS_DLG_SIZE wxSize(680,600)

SettingsDialog::SettingsDialog(wxWindow* win, GOrgueSound& sound) :
	wxPropertySheetDialog(win, wxID_ANY, _("Audio Settings"), wxDefaultPosition, SETTINGS_DLG_SIZE),
	m_Sound(sound),
	m_Settings(sound.GetSettings())
{
	b_stereo = m_Settings.GetLoadInStereo();
	m_OldBitsPerSample = m_Settings.GetBitsPerSample();
	m_OldLoopLoad = m_Settings.GetLoopLoad();
	m_OldAttackLoad = m_Settings.GetAttackLoad();
	m_OldReleaseLoad = m_Settings.GetReleaseLoad();

	CreateButtons(wxOK | wxCANCEL | wxHELP);
	//JB: wxAPPLY not available in recent versions of wxWidgets

	wxBookCtrlBase* notebook = GetBookCtrl();

	wxPanel* devices  = CreateDevicesPage(notebook);
	m_OptionsPage = new SettingsOption(m_Settings, notebook);
	m_OrganPage = new SettingsOrgan(m_Settings, notebook);
	m_MidiMessagePage = new SettingsMidiMessage(m_Settings, notebook);

	notebook->AddPage(devices,  _("Devices"));
	notebook->AddPage(m_OptionsPage,  _("Options"));
	notebook->AddPage(m_MidiMessagePage, _("Initial MIDI Configuration"));
	notebook->AddPage(m_OrganPage, _("Organs"));
}

SettingsDialog::~SettingsDialog()
{
	if ((b_stereo != m_Settings.GetLoadInStereo() || 
	     m_OldLoopLoad != m_Settings.GetLoopLoad() ||
	     m_OldAttackLoad != m_Settings.GetAttackLoad() ||
	     m_OldReleaseLoad != m_Settings.GetReleaseLoad() ||
	     m_OptionsPage->NeedReload() ||
	     m_OldBitsPerSample != m_Settings.GetBitsPerSample()) &&
	    m_Sound.GetOrganFile() != NULL)
	{
		if (::wxMessageBox(_("Some changed settings effect unless the sample set is reloaded.\n\nWould you like to reload the sample set now?"), wxT(APP_NAME), wxYES_NO | wxICON_QUESTION) == wxYES)
		{
			wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_FILE_RELOAD);
			wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
		}
	}
}

void SettingsDialog::UpdateSoundStatus()
{
	SetLatencySpinner(m_Settings.GetAudioDeviceLatency(c_sound->GetStringSelection()));
	c_format->SetLabel(wxString(GOrgueRtHelpers::GetAudioFormatName(m_Sound.GetAudioFormat())));
}

wxPanel* SettingsDialog::CreateDevicesPage(wxWindow* parent)
{
	int i;
	std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::iterator it1;
	std::map<wxString, int>::iterator it2;

	wxPanel* panel = new wxPanel(parent, wxID_ANY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString choices;

	m_Sound.GetMidi().UpdateDevices();
	for (it2 = m_Sound.GetMidi().GetDevices().begin(); it2 != m_Sound.GetMidi().GetDevices().end(); it2++)
	{
		choices.push_back(it2->first);
		page1checklistdata.push_back(m_Settings.GetMidiDeviceChannelShift(it2->first));
	}
	wxBoxSizer* item3 = new wxStaticBoxSizer(wxVERTICAL, panel, _("MIDI &input devices"));
	item0->Add(item3, 0, wxEXPAND | wxALL, 5);
	page1checklist = new wxCheckListBox(panel, ID_MIDI_DEVICES, wxDefaultPosition, wxDefaultSize, choices);
	for (i = 0; i < (int)page1checklistdata.size(); i++)
	{
        if (page1checklistdata[i] < 0)
			page1checklistdata[i] = -page1checklistdata[i] - 1;
		else
			page1checklist->Check(i);
	}
	item3->Add(page1checklist, 1, wxEXPAND | wxALL, 5);
	page1button = new wxButton(panel, ID_MIDI_PROPERTIES, _("A&dvanced..."));
	page1button->Disable();
	item3->Add(page1button, 0, wxALIGN_RIGHT | wxALL, 5);
	choices.clear();

	wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);

	for (it1 = m_Sound.GetAudioDevices().begin(); it1 != m_Sound.GetAudioDevices().end(); it1++)
		choices.push_back(it1->first);
	wxBoxSizer* item1 = new wxStaticBoxSizer(wxVERTICAL, panel, _("Sound &output device"));
	item9->Add(item1, 0, wxEXPAND | wxALL, 5);
	c_sound = new wxChoice(panel, ID_SOUND_DEVICE, wxDefaultPosition, wxDefaultSize, choices);
	c_sound->SetStringSelection(m_Sound.GetDefaultAudioDevice());
	c_sound->SetStringSelection(m_Settings.GetDefaultAudioDevice());
	item1->Add(c_sound, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Mono"));
	choices.push_back(_("Stereo"));

	wxFlexGridSizer* grid = new wxFlexGridSizer(10, 2, 5, 5);
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Output Resolution:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	grid->Add(c_format = new wxStaticText(panel, wxID_ANY, wxEmptyString));

	/* estimated latency */
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Desired &Latency:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* latency = new wxBoxSizer(wxHORIZONTAL);
	latency->Add(c_latency = new wxSpinCtrl(panel, ID_LATENCY, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 999), 0);
	latency->Add(new wxStaticText(panel, wxID_ANY, _("ms")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	grid->Add(latency, 0);

	/* achieved latency for the above estimate */
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Achieved Latency:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	grid->Add(c_actual_latency = new wxStaticText(panel, wxID_ANY, wxEmptyString));

	grid->Add(new wxStaticText(panel, wxID_ANY, _("Load &stereo samples in:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_stereo = new wxChoice(panel, ID_MONO_STEREO, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);
	item1->Add(grid, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(wxT("44100"));
	choices.push_back(wxT("48000"));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Sample Rate:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_SampleRate = new wxChoice(panel, ID_SAMPLE_RATE, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 0; i < 5; i++)
		choices.push_back(wxString::Format(wxT("%d bits"), 8 + i * 4));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Sample size:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_BitsPerSample = new wxChoice(panel, ID_BITS_PER_SAMPLE, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("First loop"));
	choices.push_back(_("Longest loop"));
	choices.push_back(_("All loops"));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Loop loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_LoopLoad = new wxChoice(panel, ID_LOOP_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("Single attack"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Attack loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_AttackLoad = new wxChoice(panel, ID_ATTACK_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("Single release"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Release loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_ReleaseLoad = new wxChoice(panel, ID_RELEASE_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("Linear"));
	choices.push_back(_("Polyphase"));
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Interpolation:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_Interpolation = new wxChoice(panel, ID_INTERPOLATION, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	UpdateSoundStatus();
	c_stereo->Select(m_Settings.GetLoadInStereo());
	c_SampleRate->Select(0);
	for(unsigned i = 0; i < c_SampleRate->GetCount(); i++)
		if (wxString::Format(wxT("%d"), m_Settings.GetSampleRate()) == c_SampleRate->GetString(i))
			c_SampleRate->Select(i);
	c_BitsPerSample->Select((m_Settings.GetBitsPerSample() - 8) / 4);
	c_LoopLoad->Select(m_Settings.GetLoopLoad());
	c_AttackLoad->Select(m_Settings.GetAttackLoad());
	c_ReleaseLoad->Select(m_Settings.GetReleaseLoad());
	c_Interpolation->Select(m_Settings.GetInterpolationType());

	topSizer->Add(item0, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	panel->SetSizer(topSizer);
	topSizer->Fit(panel);
	return panel;
}

void SettingsDialog::OnChanged(wxCommandEvent& event)
{

}

void SettingsDialog::OnDevicesSoundChoice(wxCommandEvent& event)
{

	int lat = m_Settings.GetAudioDeviceLatency(c_sound->GetStringSelection());
	SetLatencySpinner(lat);

}

void SettingsDialog::OnLatencySpinnerChange(wxSpinEvent& event)
{

	SetLatencySpinner(event.GetPosition());

}

void SettingsDialog::OnDevicesMIDIClick(wxCommandEvent& event)
{
	page1button->Enable();
}

void SettingsDialog::OnDevicesMIDIDoubleClick(wxCommandEvent& event)
{
	page1button->Enable();
	int index = page1checklist->GetSelection();
	int result = ::wxGetNumberFromUser(_("A channel offset allows the use of two MIDI\ninterfaces with conflicting MIDI channels. For\nexample, applying a channel offset of 8 to\none of the MIDI interfaces would cause that\ninterface's channel 1 to appear as channel 9,\nchannel 2 to appear as channel 10, and so on."), _("Channel offset:"), page1checklist->GetString(index), page1checklistdata[index], 0, 15, this);
	if (result >= 0 && result != page1checklistdata[index])
	{
		page1checklistdata[index] = result;
		OnChanged(event);
	}
}

void SettingsDialog::OnApply(wxCommandEvent& event)
{
	DoApply();
}

void SettingsDialog::OnHelp(wxCommandEvent& event)
{
	wxCommandEvent help(wxEVT_SHOWHELP, 0);
	help.SetString(((wxNotebook*)GetBookCtrl())->GetSelection() ? _("MIDI Settings") : _("Device Settings"));
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(help);
}

void SettingsDialog::OnOK(wxCommandEvent& event)
{
	if (DoApply())
		event.Skip();

}

bool SettingsDialog::DoApply()
{

	if (!(this->Validate()))
		return false;

	m_OptionsPage->Save();
	m_OrganPage->Save();

	for (size_t i = 0; i < page1checklist->GetCount(); i++)
	{
		int j;

		j = page1checklistdata[i];
		if (!page1checklist->IsChecked(i))
			j = -j - 1;
		m_Settings.SetMidiDeviceChannelShift(page1checklist->GetString(i), j);
	}
	m_Settings.SetDefaultAudioDevice(c_sound->GetStringSelection());
	m_Settings.SetAudioDeviceLatency(c_sound->GetStringSelection(), c_latency->GetValue());
	m_Settings.SetLoadInStereo(c_stereo->GetSelection());
	m_Settings.SetSampleRate(wxAtoi(c_SampleRate->GetStringSelection()));
	m_Settings.SetBitsPerSample(c_BitsPerSample->GetSelection() * 4 + 8);
	m_Settings.SetLoopLoad(c_LoopLoad->GetSelection());
	m_Settings.SetAttackLoad(c_AttackLoad->GetSelection());
	m_Settings.SetReleaseLoad(c_ReleaseLoad->GetSelection());
	m_Settings.SetInterpolationType(c_Interpolation->GetSelection());

	m_Sound.ResetSound();
	UpdateSoundStatus();

	return true;
}

