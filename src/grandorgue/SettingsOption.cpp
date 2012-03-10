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

#include <wx/filepicker.h>

#include "SettingsOption.h"
#include "GOrgueSettings.h"

SettingsOption::SettingsOption(GOrgueSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings)
{
	wxArrayString choices;

	m_OldSquash = m_Settings.GetLosslessCompression();

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);

	wxBoxSizer* item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Enhancements"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Squash = new wxCheckBox(this, ID_ENHANCE_SQUASH,           _("Lossless compression")          ), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Limit  = new wxCheckBox(this, ID_ENHANCE_MANAGE_POLYPHONY, _("Active polyphony management")   ), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_CompressCache  = new wxCheckBox(this, ID_COMPRESS_CACHE,    _("Compress Cache")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Scale  = new wxCheckBox(this, ID_ENHANCE_SCALE_RELEASE,    _("Release sample scaling"        )), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Random = new wxCheckBox(this, ID_ENHANCE_RANDOMIZE,        _("Randomize pipe speaking"       )), 0, wxEXPAND | wxALL, 5);
	if (m_Settings.GetLosslessCompression())
		m_Squash->SetValue(true);
	if (m_Settings.GetManagePolyphony())
		m_Limit ->SetValue(true);
	if (m_Settings.GetCompressCache())
		m_CompressCache ->SetValue(true);
	if (m_Settings.GetScaleRelease())
		m_Scale ->SetValue(true);
	if (m_Settings.GetRandomizeSpeaking())
		m_Random->SetValue(true);


	wxFlexGridSizer* grid = new wxFlexGridSizer(4, 2, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sound Engine"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	for (unsigned i = 0; i < 128; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Concurreny Level:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Concurrency = new wxChoice(this, ID_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 1; i < 8; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Release Concurreny Level:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_ReleaseConcurrency = new wxChoice(this, ID_RELEASE_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("8 Bit PCM"));
	choices.push_back(_("16 Bit PCM"));
	choices.push_back(_("24 Bit PCM"));
	choices.push_back(_("IEEE Float"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Recorder WAV Format:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_WaveFormat = new wxChoice(this, ID_WAVE_FORMAT, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	m_Concurrency->Select(m_Settings.GetConcurrency());
	m_ReleaseConcurrency->Select(m_Settings.GetReleaseConcurrency() - 1);
	m_WaveFormat->Select(m_Settings.GetWaveFormatBytesPerSample() - 1);

	grid = new wxFlexGridSizer(4, 1, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Paths"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Settings store:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SettingsPath = new wxDirPickerCtrl(this, ID_SETTINGS_DIR, wxEmptyString, _("Select directory for settings store"), wxDefaultPosition, wxDefaultSize, 
						       wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Cache store:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_CachePath = new wxDirPickerCtrl(this, ID_CACHE_DIR, wxEmptyString, _("Select directory for cache store"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 0, wxALL);
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	m_SettingsPath->SetPath(m_Settings.GetUserSettingPath());
	m_CachePath->SetPath(m_Settings.GetUserCachePath());

	topSizer->Add(item0, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsOption::Save()
{
	m_Settings.SetLosslessCompression(m_Squash->IsChecked());
	m_Settings.SetManagePolyphony(m_Limit->IsChecked());
	m_Settings.SetCompressCache(m_CompressCache->IsChecked());
	m_Settings.SetScaleRelease(m_Scale->IsChecked());
	m_Settings.SetRandomizeSpeaking(m_Random->IsChecked());
	m_Settings.SetConcurrency(m_Concurrency->GetSelection());
	m_Settings.SetReleaseConcurrency(m_ReleaseConcurrency->GetSelection() + 1);
	m_Settings.SetWaveFormatBytesPerSample(m_WaveFormat->GetSelection() + 1);
	m_Settings.SetUserSettingPath(m_SettingsPath->GetPath());
	m_Settings.SetUserCachePath(m_CachePath->GetPath());
}

bool SettingsOption::NeedReload()
{
	return m_OldSquash != m_Settings.GetLosslessCompression();
}
