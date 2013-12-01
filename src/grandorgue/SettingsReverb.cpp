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

#include "SettingsReverb.h"

#include "GOrgueSettings.h"
#include "GOrgueWave.h"
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(SettingsReverb, wxPanel)
	EVT_CHECKBOX(ID_ENABLED, SettingsReverb::OnEnabled)
	EVT_FILEPICKER_CHANGED(ID_FILE, SettingsReverb::OnFileChanged)
	EVT_TEXT(ID_GAIN, SettingsReverb::OnGainChanged)
	EVT_SPIN(ID_GAIN_SPIN, SettingsReverb::OnGainSpinChanged)
END_EVENT_TABLE()

SettingsReverb::SettingsReverb(GOrgueSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	topSizer->Add(m_Enabled = new wxCheckBox(this, ID_ENABLED, _("Enable Convolution Reverb"), wxDefaultPosition, wxDefaultSize));

	wxFlexGridSizer* grid = new wxFlexGridSizer(7, 2, 5, 5);
	wxBoxSizer* item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Convolution reverb"));
	topSizer->Add(item6, 0, wxEXPAND | wxALL, 5);

	m_File = new wxFilePickerCtrl(this, ID_FILE, wxEmptyString, _("Select an impulse file"), _("*.wav"), wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST);
	wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Impulse response:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	m_FileName = new wxStaticText(this, wxID_ANY, wxEmptyString);
	box2->Add(m_File);
	box2->Add(m_FileName);
	grid->Add(box2);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Delay (ms):")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Delay = new wxSpinCtrl(this, ID_DELAY, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Start offset:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_StartOffset = new wxSpinCtrl(this, ID_START_OFFSET, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Length:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Length = new wxSpinCtrl(this, ID_LENGTH, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Channel:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Channel = new wxChoice(this, ID_CHANNEL, wxDefaultPosition, wxDefaultSize), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Gain:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Gain = new wxTextCtrl(this, ID_GAIN, wxEmptyString);
	m_GainSpin = new wxSpinButton(this, ID_GAIN_SPIN); 
	box2->Add(m_Gain);
	box2->Add(m_GainSpin);
	grid->Add(box2);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Direct Sound:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Direct = new wxCheckBox(this, ID_DIRECT, _("Add dirac pulse"), wxDefaultPosition, wxDefaultSize));

	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);

	m_GainSpin->SetRange(1, 1000);
	m_Delay->SetRange(0, 10000);

	m_Enabled->SetValue(m_Settings.GetReverbEnabled());
	UpdateEnabled();

	m_StartOffset->SetRange(0, m_Settings.GetReverbStartOffset());
	m_Length->SetRange(0, m_Settings.GetReverbLen());

	m_File->SetPath(m_Settings.GetReverbFile());
	m_FileName->SetLabel(m_File->GetPath());
	UpdateLimits();

	m_Gain->ChangeValue(wxString::Format(wxT("%f"), m_Settings.GetReverbGain()));
	m_GainSpin->SetValue(m_Settings.GetReverbGain() * 20);
	m_StartOffset->SetValue(m_Settings.GetReverbStartOffset());
	m_Length->SetValue(m_Settings.GetReverbLen());
	m_Channel->SetSelection(m_Settings.GetReverbChannel() - 1);
	m_Direct->SetValue(m_Settings.GetReverbDirect());
	m_Delay->SetValue(m_Settings.GetReverbDelay());

	topSizer->Fit(this);
}

void SettingsReverb::UpdateLimits()
{
	if (m_File->GetPath() == wxEmptyString || !m_Enabled->GetValue())
	{
		m_StartOffset->Disable();
		m_Length->Disable();
		m_Channel->Disable();
		return;
	}
	GOrgueWave wav;
	m_FileName->SetLabel(m_File->GetPath());
	try
	{
		wav.Open(m_File->GetPath());
		m_StartOffset->Enable();
		m_Length->Enable();
		m_Channel->Enable();

		m_Channel->Clear();
		for(unsigned i = 1; i <= wav.GetChannels(); i++)
			m_Channel->Append(wxString::Format(wxT("%d"), i));
		m_Channel->SetSelection(0);
		m_StartOffset->SetRange(0, wav.GetLength());
		m_Length->SetRange(0, wav.GetLength());
	}
	catch(wxString error)
	{
		wxMessageBox(error , _("Error"), wxOK | wxICON_ERROR, this);
	}
}

void SettingsReverb::UpdateFile()
{
	UpdateLimits();
	m_Channel->SetSelection(0);
	m_StartOffset->SetValue(0);
	m_Length->SetValue(m_Length->GetMax());
}

void SettingsReverb::UpdateEnabled()
{
	if (m_Enabled->GetValue())
	{
		m_Direct->Enable();
		m_File->Enable();
		m_Channel->Enable();
		m_StartOffset->Enable();
		m_Length->Enable();
		m_Delay->Enable();
		m_Gain->Enable();
		m_GainSpin->Enable();
		UpdateLimits();
	}
	else
	{
		m_Direct->Disable();
		m_File->Disable();
		m_Channel->Disable();
		m_StartOffset->Disable();
		m_Length->Disable();
		m_Delay->Disable();
		m_Gain->Disable();
		m_GainSpin->Disable();
	}
}


void SettingsReverb::OnEnabled(wxCommandEvent& event)
{
	if (m_Enabled->GetValue())
		wxMessageBox(_("This feature is currently not supported.") , _("Warning"), wxOK | wxICON_WARNING, this);
	UpdateEnabled();
}

void SettingsReverb::OnFileChanged(wxFileDirPickerEvent& e)
{
	UpdateFile();
}

void SettingsReverb::OnGainSpinChanged(wxSpinEvent& e)
{
	m_Gain->ChangeValue(wxString::Format(wxT("%f"), (float)(m_GainSpin->GetValue() / 20.0)));
}

void SettingsReverb::OnGainChanged(wxCommandEvent &e)
{
	double gain;
	if (m_Gain->GetValue().ToDouble(&gain))
		m_GainSpin->SetValue(gain * 20);
}

void SettingsReverb::Save()
{
	m_Settings.SetReverbEnabled(m_Enabled->IsChecked());
	m_Settings.SetReverbDirect(m_Direct->IsChecked());
	m_Settings.SetReverbFile(m_File->GetPath());
	m_Settings.SetReverbLen(m_Length->GetValue());
	m_Settings.SetReverbDelay(m_Delay->GetValue());
	m_Settings.SetReverbStartOffset(m_StartOffset->GetValue());
	m_Settings.SetReverbChannel(m_Channel->GetSelection() + 1);
	double gain;
	if (m_Gain->GetValue().ToDouble(&gain))
		m_Settings.SetReverbGain(gain);
}
