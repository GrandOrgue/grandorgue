/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "SettingsDefaults.h"

#include "GOrgueSettings.h"
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

SettingsDefaults::SettingsDefaults(GOrgueSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);
	wxFlexGridSizer* grid;
	wxBoxSizer* item6;

	grid = new wxFlexGridSizer(4, 2, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Volume"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Volume:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Volume = new wxSpinCtrl(this, ID_VOLUME, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_Volume->SetRange(-120, 20);

	m_Volume->SetValue(m_Settings.Volume());
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	grid = new wxFlexGridSizer(4, 2, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Metronome"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("BPM:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_MetronomeBPM = new wxSpinCtrl(this, ID_METRONOME_BPM, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_MetronomeBPM->SetRange(1, 500);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Ticks per Measure:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_MetronomeMeasure = new wxSpinCtrl(this, ID_METRONOME_MEASURE, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_MetronomeMeasure->SetRange(0, 32);

	m_MetronomeBPM->SetValue(m_Settings.MetronomeBPM());
	m_MetronomeMeasure->SetValue(m_Settings.MetronomeMeasure());
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);

	grid = new wxFlexGridSizer(10, 1, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Paths"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Sampleset directory:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_OrganPath = new wxDirPickerCtrl(this, ID_ORGAN_PATH, wxEmptyString, _("Select directory for your samplesets"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Setting import/export directory:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SettingPath = new wxDirPickerCtrl(this, ID_SETTING_PATH, wxEmptyString, _("Select directory for setting import/export"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Audio recording directory:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_AudioRecorderPath = new wxDirPickerCtrl(this, ID_AUDIO_RECORDER_PATH, wxEmptyString, _("Select directory for your audio recordings"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("MIDI recording directory:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_MidiRecorderPath = new wxDirPickerCtrl(this, ID_MIDI_RECORDER_PATH, wxEmptyString, _("Select directory for your MIDI recording"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Midi player directory:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_MidiPlayerPath = new wxDirPickerCtrl(this, ID_MIDI_PLAYER_PATH, wxEmptyString, _("Select directory for the MIDI player"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	m_OrganPath->SetPath(m_Settings.OrganPath());
	m_SettingPath->SetPath(m_Settings.SettingPath());
	m_AudioRecorderPath->SetPath(m_Settings.AudioRecorderPath());
	m_MidiRecorderPath->SetPath(m_Settings.MidiRecorderPath());
	m_MidiPlayerPath->SetPath(m_Settings.MidiPlayerPath());

	topSizer->Add(item0, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->FitInside(this);
}

void SettingsDefaults::Save()
{
	m_Settings.OrganPath(m_OrganPath->GetPath());
	m_Settings.SettingPath(m_SettingPath->GetPath());
	m_Settings.AudioRecorderPath(m_AudioRecorderPath->GetPath());
	m_Settings.MidiRecorderPath(m_MidiRecorderPath->GetPath());
	m_Settings.MidiPlayerPath(m_MidiPlayerPath->GetPath());
	m_Settings.MetronomeBPM(m_MetronomeBPM->GetValue());
	m_Settings.MetronomeMeasure(m_MetronomeMeasure->GetValue());
	m_Settings.Volume(m_Volume->GetValue());
}
