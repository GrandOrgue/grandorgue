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
#include "SettingsMidiDevices.h"
#include "SettingsMidiMessage.h"
#include "SettingsOption.h"
#include "SettingsOrgan.h"

IMPLEMENT_CLASS(SettingsDialog, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(SettingsDialog, wxPropertySheetDialog)
	EVT_CHOICE(ID_SOUND_DEVICE, SettingsDialog::OnDevicesSoundChoice)

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

SettingsDialog::SettingsDialog(wxWindow* win, GOrgueSound& sound) :
	wxPropertySheetDialog(win, wxID_ANY, _("Audio Settings"), wxDefaultPosition, wxSize(680,600)),
	m_Sound(sound),
	m_Settings(sound.GetSettings())
{

	CreateButtons(wxOK | wxCANCEL | wxHELP);

	wxBookCtrlBase* notebook = GetBookCtrl();

	m_MidiDevicePage = new SettingsMidiDevices(m_Sound, notebook);
	wxPanel* devices  = CreateDevicesPage(notebook);
	m_OptionsPage = new SettingsOption(m_Settings, notebook);
	m_OrganPage = new SettingsOrgan(m_Settings, notebook);
	m_MidiMessagePage = new SettingsMidiMessage(m_Settings, notebook);

	notebook->AddPage(m_MidiDevicePage,  _("MIDI Devices"));
	notebook->AddPage(m_OptionsPage,  _("Options"));
	notebook->AddPage(devices,  _("Audio Output"));
	notebook->AddPage(m_MidiMessagePage, _("Initial MIDI Configuration"));
	notebook->AddPage(m_OrganPage, _("Organs"));
}

SettingsDialog::~SettingsDialog()
{
	if (m_OptionsPage->NeedReload() &&  m_Sound.GetOrganFile() != NULL)
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
	c_format->SetLabel(_("32 bit float"));
}

wxPanel* SettingsDialog::CreateDevicesPage(wxWindow* parent)
{
	std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::const_iterator it1;

	wxPanel* panel = new wxPanel(parent, wxID_ANY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString choices;

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
	item1->Add(grid, 0, wxEXPAND | wxALL, 5);

	UpdateSoundStatus();

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

void SettingsDialog::OnApply(wxCommandEvent& event)
{
	DoApply();
}

void SettingsDialog::OnHelp(wxCommandEvent& event)
{
	wxCommandEvent help(wxEVT_SHOWHELP, 0);
	help.SetString(GetBookCtrl()->GetPageText(GetBookCtrl()->GetSelection()));
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

	m_MidiDevicePage->Save();
	m_OptionsPage->Save();
	m_OrganPage->Save();

	m_Settings.SetDefaultAudioDevice(c_sound->GetStringSelection());
	m_Settings.SetAudioDeviceLatency(c_sound->GetStringSelection(), c_latency->GetValue());

	m_Sound.ResetSound();
	UpdateSoundStatus();

	return true;
}

