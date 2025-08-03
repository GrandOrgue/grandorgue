/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMidiInitial.h"

#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"
#include "gui/dialogs/midi-event/GOMidiEventDialog.h"
#include "gui/size/GOAdditionalSizeKeeperProxy.h"
#include "gui/wxcontrols/GOGrid.h"

BEGIN_EVENT_TABLE(GOSettingsMidiInitial, GODialogTab)
EVT_GRID_CMD_SELECT_CELL(ID_INITIALS, GOSettingsMidiInitial::OnInitialsSelected)
EVT_GRID_CMD_RANGE_SELECT(
  ID_INITIALS, GOSettingsMidiInitial::OnInitialsRangeSelected)
EVT_GRID_CMD_CELL_LEFT_DCLICK(
  ID_INITIALS, GOSettingsMidiInitial::OnInitialsDoubleClick)
EVT_BUTTON(ID_PROPERTIES, GOSettingsMidiInitial::OnProperties)
EVT_BUTTON(ID_DELETE, GOSettingsMidiInitial::OnDelete)
END_EVENT_TABLE()

const wxString WX_INITIALS = wxT("Initials");

enum {
  GRID_COL_GROUP = 0,
  GRID_COL_TYPE,
  GRID_COL_NAME,
  GRID_COL_MATCH,
  GRID_COL_CONFIGURED,
  GRID_N_COLS
};

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
  m_Initials->SetColSize(GRID_COL_GROUP, 150);
  m_Initials->SetColLabelValue(GRID_COL_GROUP, _("Group"));
  m_Initials->SetColSize(GRID_COL_TYPE, 100);
  m_Initials->SetColLabelValue(GRID_COL_TYPE, _("Type"));
  m_Initials->SetColSize(GRID_COL_NAME, 200);
  m_Initials->SetColLabelValue(GRID_COL_NAME, _("Name"));
  m_Initials->SetColSize(GRID_COL_MATCH, 200);
  m_Initials->SetColLabelValue(GRID_COL_MATCH, _("Matching"));
  m_Initials->SetColSize(GRID_COL_CONFIGURED, 100);
  m_Initials->SetColLabelValue(GRID_COL_CONFIGURED, _("Configured"));

  topSizer->Add(m_Initials, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *pButtons = new wxBoxSizer(wxHORIZONTAL);

  m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
  m_Properties->Disable();
  pButtons->Add(m_Properties, 0, wxALL, 5);

  m_ButtonDel = new wxButton(this, ID_DELETE, _("Delete"));
  m_ButtonDel->Disable();
  pButtons->Add(m_ButtonDel, 0, wxALL, 5);

  topSizer->Add(pButtons, 0, wxALL, 5);

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

    m_Initials->SetCellValue(i, GRID_COL_GROUP, pObj->GetObjectGroup());
    m_Initials->SetCellValue(i, GRID_COL_TYPE, pObj->GetMidiTypeName());
    m_Initials->SetCellValue(i, GRID_COL_NAME, pObj->GetName());
    m_Initials->SetCellValue(i, GRID_COL_MATCH, pObj->GetMatchingBy());
    m_Initials->SetCellValue(
      i, GRID_COL_CONFIGURED, pObj->IsMidiConfigured() ? _("Yes") : _("No"));
  }
  return true;
}

void GOSettingsMidiInitial::OnInitialsSelected(wxGridEvent &event) {
  int index = m_Initials->GetGridCursorRow();

  m_Properties->Enable(index >= 0);
}

void GOSettingsMidiInitial::OnInitialsRangeSelected(
  wxGridRangeSelectEvent &event) {
  int nInternals = GOConfig::getMidiBuiltinCount();
  wxArrayInt selectedRows = m_Initials->GetSelectedRows();
  auto it = std::find_if(
    selectedRows.begin(), selectedRows.end(), [nInternals](int x) {
      return x >= nInternals;
    });

  m_ButtonDel->Enable(it != selectedRows.end());
}

void GOSettingsMidiInitial::ConfigureInitial() {
  int index = m_Initials->GetGridCursorRow();
  GOConfigMidiObject *pObj = r_config.GetMidiInitialObject(index);
  GOMidiEventDialog dlg(
    nullptr,
    this,
    wxString::Format(_("Initial MIDI settings for %s"), pObj->GetName()),
    r_config,
    wxT("InitialSettings"),
    *pObj,
    false);

  dlg.RegisterMIDIListener(&r_midi);
  dlg.ShowModal();
  m_Initials->SetCellValue(
    index,
    GRID_COL_CONFIGURED,
    pObj->IsMidiConfigured() > 0 ? _("Yes") : _("No"));
}

void GOSettingsMidiInitial::OnDelete(wxCommandEvent &event) {
  int nInternals = GOConfig::getMidiBuiltinCount();
  wxArrayInt selectedRows = m_Initials->GetSelectedRows();
  wxArrayInt selectedUserAdded;

  // copy the user-added selected elements to selectedUserAdded
  std::copy_if(
    selectedRows.begin(),
    selectedRows.end(),
    std::back_inserter(selectedUserAdded),
    [nInternals](int x) { return x >= nInternals; });

  if (
    !selectedUserAdded.IsEmpty()
    && wxMessageBox(
         wxString::Format(
           _("Are you sure to delete %u initial MIDI objects settings?"),
           selectedUserAdded.size()),
         _("Delete initial MIDI settings"),
         wxYES_NO,
         this)
      == wxYES) {
    std::sort(selectedUserAdded.begin(), selectedUserAdded.end());

    // Do deletion in reverse order
    for (int i = selectedUserAdded.GetCount() - 1; i >= 0; i--) {
      unsigned row = (unsigned)selectedUserAdded[i];

      r_config.DelMidiInitial(row);
      m_Initials->DeleteRows(row);
    }
  }
}