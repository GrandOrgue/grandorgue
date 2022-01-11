/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsDefaults.h"

#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "config/GOConfig.h"

GOSettingsDefaults::GOSettingsDefaults(GOConfig &settings, wxWindow *parent)
  : wxPanel(parent, wxID_ANY), m_config(settings) {
  wxFlexGridSizer *const grid = new wxFlexGridSizer(1, 5, 5);
  grid->AddGrowableCol(0, 1);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ settings path:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_OrganSettingsPath = new wxDirPickerCtrl(
      this,
      ID_ORGAN_SETTINGS_PATH,
      wxEmptyString,
      _("Select directory for storing organ settings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Cache store:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_CachePath = new wxDirPickerCtrl(
      this,
      ID_ORGAN_CACHE_PATH,
      wxEmptyString,
      _("Select directory for cache store"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);
  m_OrganSettingsPath->SetPath(m_config.OrganSettingsPath());
  m_CachePath->SetPath(m_config.UserCachePath());

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Sampleset directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_OrganPath = new wxDirPickerCtrl(
      this,
      ID_ORGAN_PATH,
      wxEmptyString,
      _("Select directory for your samplesets"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ package directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_OrganPackagePath = new wxDirPickerCtrl(
      this,
      ID_ORGANPACKAGE_PATH,
      wxEmptyString,
      _("Select directory for your organ packages"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Setting import/export directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_SettingPath = new wxDirPickerCtrl(
      this,
      ID_SETTING_PATH,
      wxEmptyString,
      _("Select directory for setting import/export"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Audio recording directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_AudioRecorderPath = new wxDirPickerCtrl(
      this,
      ID_AUDIO_RECORDER_PATH,
      wxEmptyString,
      _("Select directory for your audio recordings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("MIDI recording directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_MidiRecorderPath = new wxDirPickerCtrl(
      this,
      ID_MIDI_RECORDER_PATH,
      wxEmptyString,
      _("Select directory for your MIDI recording"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Midi player directory:")),
    5,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_MidiPlayerPath = new wxDirPickerCtrl(
      this,
      ID_MIDI_PLAYER_PATH,
      wxEmptyString,
      _("Select directory for the MIDI player"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST),
    5,
    wxEXPAND | wxALL);

  this->SetSizer(grid);
  grid->FitInside(this);

  m_OrganPath->SetPath(m_config.OrganPath());
  m_OrganPackagePath->SetPath(m_config.OrganPackagePath());
  m_SettingPath->SetPath(m_config.SettingPath());
  m_AudioRecorderPath->SetPath(m_config.AudioRecorderPath());
  m_MidiRecorderPath->SetPath(m_config.MidiRecorderPath());
  m_MidiPlayerPath->SetPath(m_config.MidiPlayerPath());

}

void GOSettingsDefaults::Save() {
  m_config.OrganSettingsPath(m_OrganSettingsPath->GetPath());
  m_config.UserCachePath(m_CachePath->GetPath());
  m_config.OrganPath(m_OrganPath->GetPath());
  m_config.OrganPackagePath(m_OrganPackagePath->GetPath());
  m_config.SettingPath(m_SettingPath->GetPath());
  m_config.AudioRecorderPath(m_AudioRecorderPath->GetPath());
  m_config.MidiRecorderPath(m_MidiRecorderPath->GetPath());
  m_config.MidiPlayerPath(m_MidiPlayerPath->GetPath());
}
