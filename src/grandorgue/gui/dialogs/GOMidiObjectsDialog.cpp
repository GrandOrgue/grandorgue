/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObjectstDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>

#include "gui/size/GOAdditionalSizeKeeperProxy.h"
#include "gui/wxcontrols/GOGrid.h"
#include "midi/objects/GOMidiObject.h"

#include "GOEvent.h"

enum {
  ID_LIST = 200,
  ID_EDIT,
  ID_STATUS,
  ID_BUTTON,
  ID_BUTTON_LAST = ID_BUTTON + 2,
};

BEGIN_EVENT_TABLE(GOMidiObjectsDialog, GOSimpleDialog)
EVT_GRID_CMD_SELECT_CELL(ID_LIST, GOMidiObjectsDialog::OnSelectCell)
EVT_GRID_CMD_CELL_LEFT_DCLICK(ID_LIST, GOMidiObjectsDialog::OnObjectDoubleClick)
EVT_BUTTON(ID_EDIT, GOMidiObjectsDialog::OnConfigureButton)
EVT_BUTTON(ID_STATUS, GOMidiObjectsDialog::OnStatusButton)
EVT_COMMAND_RANGE(
  ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON, GOMidiObjectsDialog::OnActionButton)
END_EVENT_TABLE()

enum { GRID_COL_TYPE = 0, GRID_COL_ELEMENT };

GOMidiObjectsDialog::GOMidiObjectsDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  GODialogSizeSet &dialogSizes,
  const std::vector<GOMidiObject *> &midiObjects)
  : GOSimpleDialog(
    parent,
    wxT("MIDI Objects"),
    _("MIDI Objects"),
    dialogSizes,
    wxEmptyString,
    0,
    wxCLOSE | wxHELP),
    GOView(doc, this),
    r_MidiObjects(midiObjects) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_ObjectsGrid
    = new GOGrid(this, ID_LIST, wxDefaultPosition, wxSize(250, 200));
  m_ObjectsGrid->CreateGrid(0, 2, wxGrid::wxGridSelectRows);
  m_ObjectsGrid->HideRowLabels();
  m_ObjectsGrid->EnableEditing(false);
  m_ObjectsGrid->SetColLabelValue(GRID_COL_TYPE, _("Type"));
  m_ObjectsGrid->SetColLabelValue(GRID_COL_ELEMENT, _("Element"));
  m_ObjectsGrid->SetColSize(GRID_COL_TYPE, 100);
  m_ObjectsGrid->SetColSize(GRID_COL_ELEMENT, 100);

  topSizer->Add(m_ObjectsGrid, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  for (unsigned id = ID_BUTTON; id <= ID_BUTTON_LAST; id++) {
    wxButton *button = new wxButton(this, id, wxEmptyString);
    sizer->Add(button, 0, id == ID_BUTTON ? wxRESERVE_SPACE_EVEN_IF_HIDDEN : 0);
    button->Hide();
    m_ActionButtons.push_back(button);
  }
  topSizer->Add(sizer, 0, wxALIGN_LEFT | wxALL, 1);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  m_ConfigureButton = new wxButton(this, ID_EDIT, _("C&onfigure..."));
  m_ConfigureButton->Disable();
  buttons->Add(m_ConfigureButton);
  m_StatusButton = new wxButton(this, ID_STATUS, _("&Status"));
  m_StatusButton->Disable();
  buttons->Add(m_StatusButton);
  topSizer->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 1);

  topSizer->AddSpacer(5);
  LayoutWithInnerSizer(topSizer);
}

static const wxString WX_GRID_MIDI_OBJECTS = wxT("GridMidiObjects");

void GOMidiObjectsDialog::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_GRID_MIDI_OBJECTS);

  m_ObjectsGrid->ApplyColumnSizes(proxyMidiObjects);
}

void GOMidiObjectsDialog::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    sizeKeeper, WX_GRID_MIDI_OBJECTS);

  m_ObjectsGrid->CaptureColumnSizes(proxyMidiObjects);
}

bool GOMidiObjectsDialog::TransferDataToWindow() {
  unsigned oldRowCnt = m_ObjectsGrid->GetNumberRows();
  unsigned newRowCnt = r_MidiObjects.size();

  if (oldRowCnt)
    m_ObjectsGrid->DeleteRows(0, oldRowCnt);

  m_ObjectsGrid->AppendRows(newRowCnt);
  for (unsigned i = 0; i < newRowCnt; i++) {
    GOMidiObject *obj = r_MidiObjects[i];

    m_ObjectsGrid->SetCellValue(i, GRID_COL_TYPE, obj->GetMidiTypeName());
    m_ObjectsGrid->SetCellValue(i, GRID_COL_ELEMENT, obj->GetName());
  }
  return true;
}

GOMidiObject *GOMidiObjectsDialog::GetSelectedObject() const {
  return r_MidiObjects[m_ObjectsGrid->GetGridCursorRow()];
}

void GOMidiObjectsDialog::ConfigureSelectedObject() {
  GetSelectedObject()->ShowConfigDialog();
}

void GOMidiObjectsDialog::OnSelectCell(wxGridEvent &event) {
  int index = event.GetRow();
  bool isAnySelected = index >= 0;

  m_ConfigureButton->Enable(isAnySelected);
  m_StatusButton->Enable(isAnySelected);
  if (isAnySelected) {
    m_ObjectsGrid->SelectRow(index);
    GOMidiObject *obj = r_MidiObjects[index];
    std::vector<wxString> actions = obj->GetElementActions();

    for (unsigned i = 0; i < m_ActionButtons.size(); i++)
      if (i < actions.size()) {
        m_ActionButtons[i]->SetLabel(actions[i]);
        m_ActionButtons[i]->Show();
      } else
        m_ActionButtons[i]->Hide();
  }
  Layout();
  event.StopPropagation();
}

void GOMidiObjectsDialog::OnStatusButton(wxCommandEvent &event) {
  GOMidiObject *obj = GetSelectedObject();
  wxString status = obj->GetElementStatus();

  GOMessageBox(
    wxString::Format(_("Status: %s"), status),
    obj->GetMidiTypeName() + _(" ") + obj->GetName(),
    wxOK);
}

void GOMidiObjectsDialog::OnActionButton(wxCommandEvent &event) {
  GOMidiObject *obj = GetSelectedObject();
  obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}
