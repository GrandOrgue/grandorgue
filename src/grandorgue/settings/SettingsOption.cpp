/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "SettingsOption.h"

#include "sound/GOSoundDefs.h"
#include "GOChoice.h"
#include "go_limits.h"
#include "settings/GOSettings.h"
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>


SettingsOption::SettingsOption(GOSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
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

	// Language
	wxBoxSizer* item6 = new wxStaticBoxSizer(wxVERTICAL, this, wxT("&Language (need to restart)"));
	item6->Add(m_Language = new wxChoice(this, ID_LANGUAGE), 0, wxEXPAND | wxALL, 5);
	m_Language->Append("Default (" + wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()) + ")", new wxStringClientData(wxEmptyString));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_ENGLISH)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_DUTCH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_DUTCH)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_FRENCH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_FRENCH)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_GERMAN), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_GERMAN)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_ITALIAN), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_ITALIAN)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_POLISH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_POLISH)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_SPANISH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_SPANISH)));
	m_Language->Append(wxLocale::GetLanguageName(wxLANGUAGE_SWEDISH), new wxStringClientData(wxLocale::GetLanguageCanonicalName(wxLANGUAGE_SWEDISH)));
	
	m_OldLanguageCode = m_Settings.LanguageCode();
	
	unsigned langIndex = 0;
	for (unsigned i = 0; i < m_Language->GetCount(); i++)
	{
	  const wxStringClientData* const clientStr = (wxStringClientData*) m_Language->GetClientObject(i);
	  
	  if (clientStr->GetData() == m_OldLanguageCode)
	  {
	    langIndex = i;
	    break;
	  }
	}
	m_Language->SetSelection(langIndex);
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Enhancements"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Limit = new wxCheckBox(this, ID_MANAGE_POLYPHONY, _("Active polyphony management")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Scale = new wxCheckBox(this, ID_SCALE_RELEASE, _("Release sample scaling")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_Random = new wxCheckBox(this, ID_RANDOMIZE, _("Randomize pipe speaking")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_LoadLastFile  = new GOChoice<GOInitialLoadType>(this, ID_LOAD_LAST_FILE), 0, wxEXPAND | wxALL, 5);
	m_LoadLastFile->Append(_("Load last used organ at startup"), GOInitialLoadType::LOAD_LAST_USED);
	m_LoadLastFile->Append(_("Load first favorit organ at startup"), GOInitialLoadType::LOAD_FIRST);
	m_LoadLastFile->Append(_("Start without any organ"), GOInitialLoadType::LOAD_NONE);
	m_Limit->SetValue(m_Settings.ManagePolyphony());
	m_LoadLastFile->SetCurrentSelection(m_Settings.LoadLastFile());
	m_Scale->SetValue(m_Settings.ScaleRelease());
	m_Random->SetValue(m_Settings.RandomizeSpeaking());

	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 5, 5);
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sound Engine"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Linear"));
	choices.push_back(_("Polyphase"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Interpolation:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Interpolation = new wxChoice(this, ID_INTERPOLATION, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 1; i < MAX_CPU; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Number of CPU cores:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_Concurrency = new wxChoice(this, ID_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 1; i < MAX_CPU; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Workload distribution:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_ReleaseConcurrency = new wxChoice(this, ID_RELEASE_CONCURRENCY, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);

	choices.clear();
	for (unsigned i = 0; i < MAX_CPU; i++)
		choices.push_back(wxString::Format(wxT("%d"), i));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Cores used at loadtime:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
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
	m_Concurrency->Select(m_Settings.Concurrency() - 1);
	m_ReleaseConcurrency->Select(m_Settings.ReleaseConcurrency() - 1);
	m_LoadConcurrency->Select(m_Settings.LoadConcurrency());
	m_WaveFormat->Select(m_Settings.WaveFormatBytesPerSample() - 1);
	m_RecordDownmix->SetValue(m_Settings.RecordDownmix());

	item9 = new wxBoxSizer(wxVERTICAL);
	item6 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("&Paths"));
	grid = new wxFlexGridSizer(1, 5, 5);
	grid->AddGrowableCol(0, 1);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Settings store:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SettingsPath = new wxDirPickerCtrl(this, ID_SETTINGS_DIR, wxEmptyString, _("Select directory for settings store"), wxDefaultPosition, wxDefaultSize, 
						       wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 1, wxEXPAND | wxALL);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Cache store:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_CachePath = new wxDirPickerCtrl(this, ID_CACHE_DIR, wxEmptyString, _("Select directory for cache store"), wxDefaultPosition, wxDefaultSize, 
						    wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST), 1, wxEXPAND | wxALL);
	m_SettingsPath->SetPath(m_Settings.UserSettingPath());
	m_CachePath->SetPath(m_Settings.UserCachePath());

	item6->Add(grid, 1, wxEXPAND | wxALL, 5);
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	
	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sample loading"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);

	item6->Add(m_LosslessCompression = new wxCheckBox(this, ID_LOSSLESS_COMPRESSION, _("Lossless compression")), 0, wxEXPAND | wxALL, 5);
	m_LosslessCompression->SetValue(m_Settings.LosslessCompression());

	grid = new wxFlexGridSizer(2, 5, 5);
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

	item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Cache"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
	item6->Add(m_CompressCache  = new wxCheckBox(this, ID_COMPRESS_CACHE, _("Compress cache")), 0, wxEXPAND | wxALL, 5);
	item6->Add(m_ManageCache  = new wxCheckBox(this, ID_MANAGE_CACHE, _("Automatically manage cache")), 0, wxEXPAND | wxALL, 5);
	m_CompressCache->SetValue(m_Settings.CompressCache());
	m_ManageCache->SetValue(m_Settings.ManageCache());

	item9->Add(m_ODFCheck  = new wxCheckBox(this, ID_ODF_CHECK, _("Perform strict ODF")), 0, wxEXPAND | wxALL, 5);
	m_ODFCheck->SetValue(m_Settings.ODFCheck());

	item0->Add(item9, 1, wxEXPAND | wxALL, 0);
	
	topSizer->Add(item0, 0, wxEXPAND | wxALL, 5);
	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsOption::Save()
{
	if (m_Interpolation->GetSelection() == 1 && m_LosslessCompression->IsChecked())
		wxMessageBox(_("Polyphase is not supported with lossless compression - falling back to linear.") , _("Warning"), wxOK | wxICON_WARNING, this);

	m_Settings.LosslessCompression(m_LosslessCompression->IsChecked());
	m_Settings.ManagePolyphony(m_Limit->IsChecked());
	m_Settings.CompressCache(m_CompressCache->IsChecked());
	m_Settings.ManageCache(m_ManageCache->IsChecked());
	m_Settings.LoadLastFile(m_LoadLastFile->GetCurrentSelection());
	m_Settings.ODFCheck(m_ODFCheck->IsChecked());
	m_Settings.RecordDownmix(m_RecordDownmix->IsChecked());
	m_Settings.ScaleRelease(m_Scale->IsChecked());
	m_Settings.RandomizeSpeaking(m_Random->IsChecked());
	m_Settings.Concurrency(m_Concurrency->GetSelection() + 1);
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
	m_Settings.MemoryLimit(m_MemoryLimit->GetValue());
	
	// Language
	const wxStringClientData * const langData = (wxStringClientData *) m_Language->GetClientObject(m_Language->GetSelection());
	m_Settings.LanguageCode(langData->GetData());
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

bool SettingsOption::NeedRestart()
{
  return m_OldLanguageCode != m_Settings.LanguageCode();
}
