/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMidiInitial.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"
#include "gui/dialogs/midi-event/GOMidiEventDialog.h"
#include "gui/size/GOAdditionalSizeKeeperProxy.h"
#include "gui/wxcontrols/GOGrid.h"
#include "midi/elements/GOMidiReceiver.h"

BEGIN_EVENT_TABLE(GOSettingsMidiInitial, GODialogTab)
EVT_GRID_CMD_SELECT_CELL(ID_INITIALS, GOSettingsMidiInitial::OnInitialsSelected)
EVT_GRID_CMD_CELL_LEFT_DCLICK(
  ID_INITIALS, GOSettingsMidiInitial::OnInitialsDoubleClick)
EVT_BUTTON(ID_PROPERTIES, GOSettingsMidiInitial::OnProperties)
END_EVENT_TABLE()

const wxString WX_INITIALS = wxT("Initials");

enum { GRID_COL_GROUP = 0, GRID_COL_NAME, GRID_COL_CONFIGURED, GRID_N_COLS };

GOSettingsMidiInitial::GOSettingsMidiInitial(
  GOConfig &settings,
  GOMidi &midi,
  GOTabbedDialog *pDlg,
  const wxString &name,
  const wxString &label)
  : GODialogTab(pDlg, name, label), r_config(settings), r_midi(midi) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);
  topSizer->Add(
    new wxStaticText(
      this,
      wxID_ANY,
      _("Attention:\nThese initial MIDI settings only affect "
        "the first load of a organ.\nRight-click on manuals, "
        "stops, ... to do further changes.")),
    0,
    wxALL);
  topSizer->AddSpacer(5);

  m_Initials
    = new GOGrid(this, ID_INITIALS, wxDefaultPosition, wxSize(100, 40));

  m_Initials->CreateGrid(0, GRID_N_COLS, wxGrid::wxGridSelectRows);
  m_Initials->HideRowLabels();
  m_Initials->EnableEditing(false);
  m_Initials->SetColSize(GRID_COL_GROUP, 100);
  m_Initials->SetColLabelValue(GRID_COL_GROUP, _("Group"));
  m_Initials->SetColSize(GRID_COL_NAME, 150);
  m_Initials->SetColLabelValue(GRID_COL_NAME, _("Name"));
  m_Initials->SetColSize(GRID_COL_CONFIGURED, 100);
  m_Initials->SetColLabelValue(GRID_COL_CONFIGURED, _("Configured"));

  topSizer->Add(m_Initials, 1, wxEXPAND | wxALL, 5);
  m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
  m_Properties->Disable();
  topSizer->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);

  topSizer->AddSpacer(5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);
}

void GOSettingsMidiInitial::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyGrid(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_INITIALS);

  m_Initials->ApplyColumnSizes(proxyGrid);
}

void GOSettingsMidiInitial::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyGrid(sizeKeeper, WX_INITIALS);

  m_Initials->CaptureColumnSizes(proxyGrid);
}

bool GOSettingsMidiInitial::TransferDataToWindow() {
  const unsigned nInitials = r_config.GetMidiInitialCount();

  m_Initials->ClearGrid();
  m_Initials->AppendRows(nInitials);
  for (unsigned i = 0; i < nInitials; i++) {
    const GOConfigMidiObject *pObj = r_config.GetMidiInitialObject(i);

    m_Initials->SetCellValue(i, GRID_COL_GROUP, r_config.GetEventGroup(i));
    m_Initials->SetCellValue(i, GRID_COL_NAME, r_config.GetEventTitle(i));
    m_Initials->SetCellValue(
      i, GRID_COL_CONFIGURED, pObj->IsMidiConfigured() ? _("Yes") : _("No"));
  }
  return true;
}

void GOSettingsMidiInitial::OnInitialsSelected(wxGridEvent &event) {
  int index = m_Initials->GetGridCursorRow();

  m_Properties->Enable(index >= 0);
}

void GOSettingsMidiInitial::ConfigureInitial() {
  int index = m_Initials->GetGridCursorRow();
  GOConfigMidiObject *pObj = r_config.GetMidiInitialObject(index);
  GOMidiEventDialog dlg(
    this,
    wxString::Format(
      _("Initial MIDI settings for %s"), r_config.GetEventTitle(index)),
    r_config,
    wxT("InitialSettings"),
    pObj->GetMidiReceiver());

  dlg.RegisterMIDIListener(&r_midi);
  dlg.ShowModal();
  m_Initials->SetCellValue(
    index, GRID_COL_CONFIGURED, pObj->IsMidiConfigured() ? _("Yes") : _("No"));
}
