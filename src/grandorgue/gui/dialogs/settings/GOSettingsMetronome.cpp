/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMetronome.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"

enum {
  ID_METRONOME_MEASURE = 200,
  ID_METRONOME_BPM,
};

static const wxSize SPINCTRL_SIZE(120, wxDefaultCoord);

GOSettingsMetronome::GOSettingsMetronome(
  GOConfig &config,
  GOTabbedDialog *pDlg,
  const wxString &name,
  const wxString &label)
  : GODialogTab(pDlg, name, label), r_config(config) {
  wxGridBagSizer *const gbSizer = new wxGridBagSizer(5, 5);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("BPM:")),
    wxGBPosition(0, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  gbSizer->Add(
    m_MetronomeBPM = new wxSpinCtrl(
      this, ID_METRONOME_BPM, wxEmptyString, wxDefaultPosition, SPINCTRL_SIZE),
    wxGBPosition(0, 1),
    wxDefaultSpan,
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  m_MetronomeBPM->SetRange(1, 500);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Ticks per Measure:")),
    wxGBPosition(1, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  gbSizer->Add(
    m_MetronomeMeasure = new wxSpinCtrl(
      this,
      ID_METRONOME_MEASURE,
      wxEmptyString,
      wxDefaultPosition,
      SPINCTRL_SIZE),
    wxGBPosition(1, 1),
    wxDefaultSpan,
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  m_MetronomeMeasure->SetRange(0, 32);

  SetSizerAndFit(gbSizer);
}

bool GOSettingsMetronome::TransferDataToWindow() {
  m_MetronomeBPM->SetValue(r_config.MetronomeBPM());
  m_MetronomeMeasure->SetValue(r_config.MetronomeMeasure());
  return true;
}

bool GOSettingsMetronome::TransferDataFromWindow() {
  r_config.MetronomeBPM(m_MetronomeBPM->GetValue());
  r_config.MetronomeMeasure(m_MetronomeMeasure->GetValue());
  return true;
}
