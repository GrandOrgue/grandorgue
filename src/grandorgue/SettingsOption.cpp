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

#include "SettingsOption.h"

#include "GOSoundDefs.h"
#include "GOrgueLimits.h"
#include "GOrgueSettings.h"
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

SettingsOption::SettingsOption(GOrgueSettings& settings, wxWindow* parent) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL),
	m_Settings(settings)
{
	wxArrayString choices;

	m_OldChannels = m_Settings.LoadChannels();
	m_OldLosslessCompression = m_Settings.LosslessCompression();
	m_OldBitsPerSample = m_Settings.BitsPerSample();
	m_OldLoopLoad = m_Settings.LoopLoad();
	m_OldAttackLoad = m_Settings.AttackLoad();
	m_OldReleaseLoad = m_Settings.ReleaseLoad();

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);

	wxBoxSizer* item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Enhancements"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Limit = new wxCheckBox(this, ID_MANAGE_POLYPHONY, _("Active polyphony management")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Scale = new wxCheckBox(this, ID_SCALE_RELEASE, _("Release sample scaling")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Random = new wxCheckBox(this, ID_RANDOMIZE, _("Randomize pipe speaking")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_LoadLastFile  = new wxCheckBox(this, ID_LOAD_LAST_FILE, _("Load last file at startup")), 0, wxEXPAND | wxALL, 5);
	m_Limit->SetValue(m_Settings.ManagePolyphony());
	m_LoadLastFile->SetValue(m_Settings.LoadLastFile());
	m_Scale->SetValue(m_Settings.ScaleRelease());
	m_Random->SetValue(m_Settings.RandomizeSpeaking());

	wxFlexGridSizer* grid = new wxFlexGridSizer(5, 2, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sound Engine"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Linear"));
	choices.push_back(_("Polyphase"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Interpolation:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Interpolation = new wxChoice(this, ID_INTERPOLATION, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 0; i < MAX_CPU; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Concurrency Level:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Concurrency = new wxChoice(this, ID_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 1; i < 8; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Release Concurrency Level:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_ReleaseConcurrency = new wxChoice(this, ID_RELEASE_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 0; i < MAX_CPU; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Load Concurrency Level:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_LoadConcurrency = new wxChoice(this, ID_LOAD_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("8 Bit PCM"));
	choices.push_back(_("16 Bit PCM"));
	choices.push_back(_("24 Bit PCM"));
	choices.push_back(_("IEEE Float"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Recorder WAV Format:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_WaveFormat = new wxChoice(this, ID_WAVE_FORMAT, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_RecordDownmix  = new wxCheckBox(this, ID_RECORD_DOWNMIX, _("Record stereo downmix")), 0, wxEXPAND | wxALL, 5);

	m_Interpolation->Select(m_Settings.InterpolationType());
	m_Concurrency->Select(m_Settings.Concurrency());
	m_ReleaseConcurrency->Select(m_Settings.ReleaseConcurrency() - 1);
	m_LoadConcurrency->Select(m_Settings.LoadConcurrency());
	m_WaveFormat->Select(m_Settings.WaveFormatBytesPerSample() - 1);
	m_RecordDownmix->SetValue(m_Settings.RecordDownmix());

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

	m_SettingsPath->SetPath(m_Settings.UserSettingPath());
	m_CachePath->SetPath(m_Settings.UserCachePath());

	item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);
	
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sample loading"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	item6->Add(m_LosslessCompression = new wxCheckBox(this, ID_LOSSLESS_COMPRESSION, _("Lossless compression")), 0, wxEXPAND | wxALL, 5);
	m_LosslessCompression->SetValue(m_Settings.LosslessCompression());

	grid = new wxFlexGridSizer(6, 2, 5, 5);
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Don't load"));
	choices.push_back(_("Mono"));
	choices.push_back(_("Stereo"));	
	grid->Add(new wxStaticText(this, wxID_ANY, _("Load &stereo samples in:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Channels = new wxChoice(this, ID_CHANNELS, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 0; i < 5; i++)
		choices.push_back(wxString::Format(wxT("%d bits"), 8 + i * 4));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Sample size:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_BitsPerSample = new wxChoice(this, ID_BITS_PER_SAMPLE, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("First loop"));
	choices.push_back(_("Longest loop"));
	choices.push_back(_("All loops"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Loop loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_LoopLoad = new wxChoice(this, ID_LOOP_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("Single attack"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Attack loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_AttackLoad = new wxChoice(this, ID_ATTACK_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	choices.push_back(_("Single release"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Release loading:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_ReleaseLoad = new wxChoice(this, ID_RELEASE_LOAD, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Memory Limit (MB):")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_MemoryLimit = new wxSpinCtrl(this, ID_MEMORY_LIMIT, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_MemoryLimit->SetRange(0, 1024 * 1024);

	m_Channels->Select(m_Settings.LoadChannels());
	m_BitsPerSample->Select((m_Settings.BitsPerSample() - 8) / 4);
	m_LoopLoad->Select(m_Settings.LoopLoad());
	m_AttackLoad->Select(m_Settings.AttackLoad());
	m_ReleaseLoad->Select(m_Settings.ReleaseLoad());
	m_MemoryLimit->SetValue(m_Settings.MemoryLimit());

	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sound output"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	grid = new wxFlexGridSizer(4, 2, 5, 5);
	item6->Add(grid, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(wxT("44100"));
	choices.push_back(wxT("48000"));
	choices.push_back(wxT("96000"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Sample Rate:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SampleRate = new wxChoice(this, ID_SAMPLE_RATE, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Samples per buffer:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SamplesPerBuffer = new wxSpinCtrl(this, ID_SAMPLES_PER_BUFFER, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_SamplesPerBuffer->SetRange(1, MAX_FRAME_SIZE);
	m_SamplesPerBuffer->SetValue(m_Settings.SamplesPerBuffer());

	m_SampleRate->Select(0);
	for(unsigned i = 0; i < m_SampleRate->GetCount(); i++)
		if (wxString::Format(wxT("%d"), m_Settings.SampleRate()) == m_SampleRate->GetString(i))
			m_SampleRate->Select(i);

	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Cache"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_CompressCache  = new wxCheckBox(this, ID_COMPRESS_CACHE, _("Compress cache")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_ManageCache  = new wxCheckBox(this, ID_MANAGE_CACHE, _("Automatically manage cache")), 0, wxEXPAND | wxALL, 5);
	m_CompressCache->SetValue(m_Settings.CompressCache());
	m_ManageCache->SetValue(m_Settings.ManageCache());

	item9->Add(m_ODFCheck  = new wxCheckBox(this, ID_ODF_CHECK, _("Perform strict ODF")), 0, wxEXPAND | wxALL, 5);
	m_ODFCheck->SetValue(m_Settings.ODFCheck());

	topSizer->Add(item0, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	this->SetScrollRate(5, 5);
	this->SetSizer(topSizer);
	topSizer->FitInside(this);
}

void SettingsOption::Save()
{
	if (m_Interpolation->GetSelection() == 1 && m_LosslessCompression->IsChecked())
		wxMessageBox(_("Polyphase is not supported with lossless compression - falling back to linear.") , _("Warning"), wxOK | wxICON_WARNING, this);

	m_Settings.LosslessCompression(m_LosslessCompression->IsChecked());
	m_Settings.ManagePolyphony(m_Limit->IsChecked());
	m_Settings.CompressCache(m_CompressCache->IsChecked());
	m_Settings.ManageCache(m_ManageCache->IsChecked());
	m_Settings.LoadLastFile(m_LoadLastFile->IsChecked());
	m_Settings.ODFCheck(m_ODFCheck->IsChecked());
	m_Settings.RecordDownmix(m_RecordDownmix->IsChecked());
	m_Settings.ScaleRelease(m_Scale->IsChecked());
	m_Settings.RandomizeSpeaking(m_Random->IsChecked());
	m_Settings.Concurrency(m_Concurrency->GetSelection());
	m_Settings.ReleaseConcurrency(m_ReleaseConcurrency->GetSelection() + 1);
	m_Settings.LoadConcurrency(m_LoadConcurrency->GetSelection());
	m_Settings.WaveFormatBytesPerSample(m_WaveFormat->GetSelection() + 1);
	m_Settings.UserSettingPath(m_SettingsPath->GetPath());
	m_Settings.UserCachePath(m_CachePath->GetPath());
	m_Settings.BitsPerSample(m_BitsPerSample->GetSelection() * 4 + 8);
	m_Settings.LoopLoad(m_LoopLoad->GetSelection());
	m_Settings.AttackLoad(m_AttackLoad->GetSelection());
	m_Settings.ReleaseLoad(m_ReleaseLoad->GetSelection());
	m_Settings.LoadChannels(m_Channels->GetSelection());
	m_Settings.InterpolationType(m_Interpolation->GetSelection());
	unsigned long sample_rate;
	if (m_SampleRate->GetStringSelection().ToULong(&sample_rate))
		m_Settings.SampleRate(sample_rate);
	else
		wxLogError(_("Invalid sample rate"));
	m_Settings.SamplesPerBuffer(m_SamplesPerBuffer->GetValue());
	m_Settings.MemoryLimit(m_MemoryLimit->GetValue());
}

bool SettingsOption::NeedReload()
{
	return m_OldLosslessCompression != m_Settings.LosslessCompression() ||
		m_OldBitsPerSample != m_Settings.BitsPerSample() ||
		m_OldLoopLoad != m_Settings.LoopLoad() || 
		m_OldAttackLoad != m_Settings.AttackLoad() ||
		m_OldReleaseLoad != m_Settings.ReleaseLoad() ||
		m_OldChannels != m_Settings.LoadChannels();
}
