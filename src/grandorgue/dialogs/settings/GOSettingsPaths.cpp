/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsPaths.h"

#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "config/GOConfig.h"
#include "wxcontrols/GODirPickerCtrl.h"

GOSettingsPaths::GOSettingsPaths(GOConfig &settings, wxWindow *parent)
  : wxPanel(parent, wxID_ANY), m_config(settings) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxFlexGridSizer *const grid = new wxFlexGridSizer(2, 5, 5);

  grid->AddGrowableCol(1, 1);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Samplesets:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_Samplesets = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for open new organs at"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ packages:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_OrganPackages = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for load new organ packages from"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ cache:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_OrganCache = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for storing organ cache"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Organ settings:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_OrganSettings = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for storing organ settings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Combinations:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_OrganCombinations = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for export/import organ combinations"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Export/import:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_ExportImport = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for export/import organ settings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Audio recording:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_AudioRecorder = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for your audio recordings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("MIDI recording:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_MidiRecorder = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for your MIDI recording"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Midi player:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_MidiPlayer = new GODirPickerCtrl(
      this,
      wxID_ANY,
      _("Select directory for the MIDI player"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST | wxDIRP_SMALL),
    0,
    wxEXPAND);

  topSizer->Add(grid, 1, wxALL, 5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);

  m_Samplesets->SetPath(m_config.OrganPath());
  m_OrganPackages->SetPath(m_config.OrganPackagePath());
  m_OrganCache->SetPath(m_config.OrganCachePath());
  m_OrganSettings->SetPath(m_config.OrganSettingsPath());
  m_OrganCombinations->SetPath(m_config.OrganCombinationsPath());
  m_ExportImport->SetPath(m_config.ExportImportPath());
  m_AudioRecorder->SetPath(m_config.AudioRecorderPath());
  m_MidiRecorder->SetPath(m_config.MidiRecorderPath());
  m_MidiPlayer->SetPath(m_config.MidiPlayerPath());
}

bool GOSettingsPaths::TransferDataFromWindow() {
  m_config.OrganPath(m_Samplesets->GetPath());
  m_config.OrganPackagePath(m_OrganPackages->GetPath());
  m_config.OrganCachePath(m_OrganCache->GetPath());
  m_config.OrganSettingsPath(m_OrganSettings->GetPath());
  m_config.OrganCombinationsPath(m_OrganCombinations->GetPath());
  m_config.ExportImportPath(m_ExportImport->GetPath());
  m_config.AudioRecorderPath(m_AudioRecorder->GetPath());
  m_config.MidiRecorderPath(m_MidiRecorder->GetPath());
  m_config.MidiPlayerPath(m_MidiPlayer->GetPath());
  return true;
}
