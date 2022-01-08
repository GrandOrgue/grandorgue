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

#include "config/GOConfig.h"

GOSettingsDefaults::GOSettingsDefaults(GOConfig &settings, wxWindow *parent)
    : wxPanel(parent, wxID_ANY), m_config(settings) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *item0 = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer *item9 = new wxBoxSizer(wxVERTICAL);
  item0->Add(item9, 0, wxEXPAND | wxALL, 0);
  wxFlexGridSizer *grid;
  wxBoxSizer *item6;

  grid = new wxFlexGridSizer(2, 5, 5);
  item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Volume"));
  item9->Add(item6, 0, wxEXPAND | wxALL, 5);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Volume:")),
    0,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_Volume = new wxSpinCtrl(
      this, ID_VOLUME, wxEmptyString, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);
  m_Volume->SetRange(-120, 20);

  m_Volume->SetValue(m_config.Volume());
  item6->Add(grid, 0, wxEXPAND | wxALL, 5);

  grid = new wxFlexGridSizer(2, 5, 5);
  item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Metronome"));
  item9->Add(item6, 0, wxEXPAND | wxALL, 5);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("BPM:")),
    0,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_MetronomeBPM = new wxSpinCtrl(
      this, ID_METRONOME_BPM, wxEmptyString, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);
  m_MetronomeBPM->SetRange(1, 500);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Ticks per Measure:")),
    0,
    wxALL | wxALIGN_CENTER_VERTICAL);
  grid->Add(
    m_MetronomeMeasure = new wxSpinCtrl(
      this,
      ID_METRONOME_MEASURE,
      wxEmptyString,
      wxDefaultPosition,
      wxDefaultSize),
    0,
    wxALL);
  m_MetronomeMeasure->SetRange(0, 32);

  m_MetronomeBPM->SetValue(m_config.MetronomeBPM());
  m_MetronomeMeasure->SetValue(m_config.MetronomeMeasure());
  item6->Add(grid, 0, wxEXPAND | wxALL, 5);

  item9 = new wxBoxSizer(wxVERTICAL);
  item0->Add(item9, 1, wxEXPAND | wxALL, 0);

  grid = new wxFlexGridSizer(1, 5, 5);
  grid->AddGrowableCol(0, 0);
  item6 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Paths"));
  item9->Add(item6, 1, wxEXPAND | wxALL, 5);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Sampleset directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ package directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Setting import/export directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Audio recording directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("MIDI recording directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Midi player directory:")),
    0,
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
    1,
    wxEXPAND | wxALL);

  item6->Add(grid, 1, wxEXPAND | wxALL, 5);

  m_OrganPath->SetPath(m_config.OrganPath());
  m_OrganPackagePath->SetPath(m_config.OrganPackagePath());
  m_SettingPath->SetPath(m_config.SettingPath());
  m_AudioRecorderPath->SetPath(m_config.AudioRecorderPath());
  m_MidiRecorderPath->SetPath(m_config.MidiRecorderPath());
  m_MidiPlayerPath->SetPath(m_config.MidiPlayerPath());

  topSizer->Add(item0, 1, wxEXPAND | wxALL, 5);
  topSizer->AddSpacer(5);
  this->SetSizer(topSizer);
  topSizer->FitInside(this);
}

void GOSettingsDefaults::Save() {
  m_config.OrganPath(m_OrganPath->GetPath());
  m_config.OrganPackagePath(m_OrganPackagePath->GetPath());
  m_config.SettingPath(m_SettingPath->GetPath());
  m_config.AudioRecorderPath(m_AudioRecorderPath->GetPath());
  m_config.MidiRecorderPath(m_MidiRecorderPath->GetPath());
  m_config.MidiPlayerPath(m_MidiPlayerPath->GetPath());
  m_config.MetronomeBPM(m_MetronomeBPM->GetValue());
  m_config.MetronomeMeasure(m_MetronomeMeasure->GetValue());
  m_config.Volume(m_Volume->GetValue());
}
