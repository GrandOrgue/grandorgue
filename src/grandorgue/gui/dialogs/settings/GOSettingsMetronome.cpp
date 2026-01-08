/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMetronome.h"

#include <wx/gbsizer.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"
#include "gui/wxcontrols/GOFilePickerCtrl.h"

static const wxSize SPINCTRL_SIZE(120, wxDefaultCoord);

static const wxString WX_SOUND_CHOICES[]
  = {_("Ringing"), _("Clicking"), _("Custom")};
static constexpr size_t SOUND_CHOICE_CNT
  = sizeof(WX_SOUND_CHOICES) / sizeof(wxString);
static const wxString WX_FILE_MASK_SAMPLES
  = wxT("Audio sample files (*.wav;*.wv)|*.wav;*.wv");

enum { ID_MEASURE = 200, ID_BPM, ID_SOUND, ID_FIRST_BEAT_PATH, ID_BEAT_PATH };

GOSettingsMetronome::GOSettingsMetronome(
  GOConfig &config,
  GOTabbedDialog *pDlg,
  const wxString &name,
  const wxString &label)
  : GODialogTab(pDlg, name, label), r_config(config) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxGridBagSizer *const gbSizer = new wxGridBagSizer(5, 5);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("BPM:")),
    wxGBPosition(0, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_bpm = new wxSpinCtrl(
    this, ID_BPM, wxEmptyString, wxDefaultPosition, SPINCTRL_SIZE);
  gbSizer->Add(
    m_bpm,
    wxGBPosition(0, 2),
    wxDefaultSpan,
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  m_bpm->SetRange(1, 500);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Ticks per Measure:")),
    wxGBPosition(1, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_measure = new wxSpinCtrl(
    this, ID_MEASURE, wxEmptyString, wxDefaultPosition, SPINCTRL_SIZE);
  gbSizer->Add(
    m_measure,
    wxGBPosition(1, 2),
    wxDefaultSpan,
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  m_measure->SetRange(0, 32);
  m_sound = new wxRadioBox(
    this,
    ID_SOUND,
    _("Metronome Sound"),
    wxDefaultPosition,
    wxDefaultSize,
    SOUND_CHOICE_CNT,
    WX_SOUND_CHOICES,
    0,
    wxRA_SPECIFY_ROWS);
  gbSizer->Add(m_sound, wxGBPosition(2, 0), wxGBSpan(1, 3), wxEXPAND);
  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Custom sound wave paths:")),
    wxGBPosition(3, 0),
    wxGBSpan(1, 3),
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("First beat:")),
    wxGBPosition(4, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_FirstBeatPath = new GOFilePickerCtrl(
    this,
    ID_FIRST_BEAT_PATH,
    _("Select a sample file for the first metronome beat"),
    WX_FILE_MASK_SAMPLES,
    wxDefaultPosition,
    wxDefaultSize,
    wxFLP_USE_TEXTCTRL | wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_SMALL);
  gbSizer->Add(m_FirstBeatPath, wxGBPosition(4, 2), wxDefaultSpan, wxEXPAND);
  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Beat:")),
    wxGBPosition(5, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_BeatPath = new GOFilePickerCtrl(
    this,
    ID_BEAT_PATH,
    _("Select a sample file for all other metronome beats"),
    WX_FILE_MASK_SAMPLES,
    wxDefaultPosition,
    wxDefaultSize,
    wxFLP_USE_TEXTCTRL | wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_SMALL);
  gbSizer->Add(m_BeatPath, wxGBPosition(5, 2), wxDefaultSpan, wxEXPAND);
  gbSizer->AddGrowableCol(2, 1);

  topSizer->Add(gbSizer, 1, wxEXPAND | wxALL, 5);
  SetSizerAndFit(topSizer);
}

bool GOSettingsMetronome::TransferDataToWindow() {
  m_bpm->SetValue(r_config.MetronomeBPM());
  m_measure->SetValue(r_config.MetronomeMeasure());
  return true;
}

bool GOSettingsMetronome::TransferDataFromWindow() {
  r_config.MetronomeBPM(m_bpm->GetValue());
  r_config.MetronomeMeasure(m_measure->GetValue());
  return true;
}
