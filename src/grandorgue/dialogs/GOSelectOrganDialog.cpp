/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSelectOrganDialog.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "size/GOAdditionalSizeKeeperProxy.h"
#include "wxcontrols/GOGrid.h"

#include "GOOrgan.h"
#include "GOOrganList.h"

enum { ID_ORGANS = 200 };

BEGIN_EVENT_TABLE(GOSelectOrganDialog, GOSimpleDialog)
EVT_GRID_CMD_SELECT_CELL(ID_ORGANS, GOSelectOrganDialog::OnSelectCell)
EVT_GRID_CMD_CELL_LEFT_DCLICK(ID_ORGANS, GOSelectOrganDialog::OnDoubleClick)
END_EVENT_TABLE()

enum { GRID_COL_NAME = 0, GRID_COL_PACKAGE, GRID_COL_PATH, GRID_N_COLS };

GOSelectOrganDialog::GOSelectOrganDialog(wxWindow *parent, GOConfig &config)
  : GOSimpleDialog(
    nullptr,
    wxT("Load"),
    _("Select organ to load"),
    config.m_DialogSizes,
    wxEmptyString,
    wxDIALOG_NO_PARENT),
    r_OrganList(config) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_GridOrgans
    = new GOGrid(this, ID_ORGANS, wxDefaultPosition, wxSize(100, 40));
  m_GridOrgans->CreateGrid(0, 3, wxGrid::wxGridSelectRows);
  m_GridOrgans->HideRowLabels();
  m_GridOrgans->EnableEditing(false);
  m_GridOrgans->SetColLabelValue(GRID_COL_NAME, _("Name"));
  m_GridOrgans->SetColLabelValue(GRID_COL_PACKAGE, _("Organ package"));
  m_GridOrgans->SetColLabelValue(GRID_COL_PATH, _("Organ path"));
  m_GridOrgans->SetColSize(GRID_COL_NAME, 150);
  m_GridOrgans->SetColSize(GRID_COL_PACKAGE, 150);
  m_GridOrgans->SetColSize(GRID_COL_PATH, 300);
  m_GridOrgans->SetColumnRightVisible(GRID_COL_PATH, true);
  topSizer->Add(m_GridOrgans, 1, wxEXPAND | wxALL, 5);

  LayoutWithInnerSizer(topSizer);
}

const wxString WX_GRID_ORGANS = wxT("GridOrgans");

void GOSelectOrganDialog::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyGridOrgans(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_GRID_ORGANS);

  m_GridOrgans->ApplyColumnSizes(proxyGridOrgans);
}

void GOSelectOrganDialog::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyGridOrgans(sizeKeeper, WX_GRID_ORGANS);

  m_GridOrgans->CaptureColumnSizes(proxyGridOrgans);
}

bool GOSelectOrganDialog::TransferDataToWindow() {
  int nRowsOld = m_GridOrgans->GetNumberRows();
  unsigned rowN = 0;

  if (nRowsOld > 0)
    m_GridOrgans->DeleteRows(0, nRowsOld);
  m_OrganPtrs.clear();
  for (const auto o : r_OrganList.GetOrganList())
    if (o->IsUsable(r_OrganList)) {
      const wxString &archiveId = o->GetArchiveID();
      const bool isArchive = !archiveId.IsEmpty();
      wxString packageName;

      if (isArchive) {
        const GOArchiveFile *a
          = r_OrganList.GetArchiveByPath(o->GetArchivePath());

        packageName = a ? a->GetName() : archiveId;
      }
      m_OrganPtrs.push_back(o);
      m_GridOrgans->AppendRows(1);
      m_GridOrgans->SetCellValue(rowN, GRID_COL_NAME, o->GetChurchName());
      m_GridOrgans->SetCellValue(rowN, GRID_COL_PACKAGE, packageName);
      m_GridOrgans->SetCellValue(
        rowN, GRID_COL_PATH, isArchive ? o->GetArchivePath() : o->GetODFPath());
      rowN++;
    }
  return true;
}

void GOSelectOrganDialog::OnSelectCell(wxGridEvent &event) {
  const int index = event.GetRow();
  const wxArrayInt selectedRows = m_GridOrgans->GetSelectedRows();
  bool isAlreadySelected = false;

  for (auto rowN : selectedRows)
    if (rowN == index)
      isAlreadySelected = true;
    else
      m_GridOrgans->DeselectRow(rowN);
  if (index >= 0 && !isAlreadySelected)
    m_GridOrgans->SelectRow(index);
}

const GOOrgan *GOSelectOrganDialog::GetSelection() {
  int index = m_GridOrgans->GetGridCursorRow();

  return index >= 0 ? m_OrganPtrs[index] : nullptr;
}

bool GOSelectOrganDialog::Validate() {
  bool isValid = true;

  if (!GetSelection()) {
    wxMessageBox(
      _("Please select an organ"), _("Error"), wxOK | wxICON_ERROR, this);
    isValid = false;
  }
  return isValid;
}
