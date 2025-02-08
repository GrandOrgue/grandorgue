/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiListDialog.h"

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

BEGIN_EVENT_TABLE(GOMidiListDialog, GOSimpleDialog)
EVT_GRID_CMD_SELECT_CELL(ID_LIST, GOMidiListDialog::OnObjectClick)
EVT_GRID_CMD_CELL_LEFT_DCLICK(ID_LIST, GOMidiListDialog::OnObjectDoubleClick)
EVT_BUTTON(ID_STATUS, GOMidiListDialog::OnStatus)
EVT_BUTTON(ID_EDIT, GOMidiListDialog::OnEdit)
EVT_COMMAND_RANGE(
  ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON, GOMidiListDialog::OnButton)
END_EVENT_TABLE()

enum { GRID_COL_TYPE = 0, GRID_COL_ELEMENT };

GOMidiListDialog::GOMidiListDialog(
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

  m_Objects = new GOGrid(this, ID_LIST, wxDefaultPosition, wxSize(250, 200));
  m_Objects->CreateGrid(0, 2, wxGrid::wxGridSelectRows);
  m_Objects->HideRowLabels();
  m_Objects->EnableEditing(false);
  m_Objects->SetColLabelValue(GRID_COL_TYPE, _("Type"));
  m_Objects->SetColLabelValue(GRID_COL_ELEMENT, _("Element"));
  m_Objects->SetColSize(GRID_COL_TYPE, 100);
  m_Objects->SetColSize(GRID_COL_ELEMENT, 100);

  topSizer->Add(m_Objects, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  for (unsigned id = ID_BUTTON; id <= ID_BUTTON_LAST; id++) {
    wxButton *button = new wxButton(this, id, wxEmptyString);
    sizer->Add(button, 0, id == ID_BUTTON ? wxRESERVE_SPACE_EVEN_IF_HIDDEN : 0);
    button->Hide();
    m_Buttons.push_back(button);
  }
  topSizer->Add(sizer, 0, wxALIGN_LEFT | wxALL, 1);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  m_Edit = new wxButton(this, ID_EDIT, _("C&onfigure..."));
  m_Edit->Disable();
  buttons->Add(m_Edit);
  m_Status = new wxButton(this, ID_STATUS, _("&Status"));
  m_Status->Disable();
  buttons->Add(m_Status);
  topSizer->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 1);

  topSizer->AddSpacer(5);
  LayoutWithInnerSizer(topSizer);
}

static const wxString WX_GRID_MIDI_OBJECTS = wxT("GridMidiObjects");

void GOMidiListDialog::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_GRID_MIDI_OBJECTS);

  m_Objects->ApplyColumnSizes(proxyMidiObjects);
}

void GOMidiListDialog::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    sizeKeeper, WX_GRID_MIDI_OBJECTS);

  m_Objects->CaptureColumnSizes(proxyMidiObjects);
}

bool GOMidiListDialog::TransferDataToWindow() {
  unsigned oldRowCnt = m_Objects->GetNumberRows();
  unsigned newRowCnt = r_MidiObjects.size();

  if (oldRowCnt)
    m_Objects->DeleteRows(0, oldRowCnt);

  m_Objects->AppendRows(newRowCnt);
  for (unsigned i = 0; i < newRowCnt; i++) {
    GOMidiObject *obj = r_MidiObjects[i];

    m_Objects->SetCellValue(i, GRID_COL_TYPE, obj->GetMidiType());
    m_Objects->SetCellValue(i, GRID_COL_ELEMENT, obj->GetMidiName());
  }
  return true;
}

GOMidiObject *GOMidiListDialog::GetSelectedObject() const {
  return r_MidiObjects[m_Objects->GetGridCursorRow()];
}

void GOMidiListDialog::OnButton(wxCommandEvent &event) {
  GOMidiObject *obj = GetSelectedObject();
  obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}

void GOMidiListDialog::OnObjectClick(wxGridEvent &event) {
  int index = event.GetRow();
  bool isAnySelected = index >= 0;

  m_Edit->Enable(isAnySelected);
  m_Status->Enable(isAnySelected);
  if (isAnySelected) {
    m_Objects->SelectRow(index);
    GOMidiObject *obj = r_MidiObjects[index];
    std::vector<wxString> actions = obj->GetElementActions();

    for (unsigned i = 0; i < m_Buttons.size(); i++)
      if (i < actions.size()) {
        m_Buttons[i]->SetLabel(actions[i]);
        m_Buttons[i]->Show();
      } else
        m_Buttons[i]->Hide();
  }
  Layout();
  event.StopPropagation();
}

void GOMidiListDialog::OnObjectDoubleClick(wxGridEvent &event) {
  GOMidiObject *obj = GetSelectedObject();

  obj->ShowConfigDialog();
}

void GOMidiListDialog::OnStatus(wxCommandEvent &event) {
  GOMidiObject *obj = GetSelectedObject();
  wxString status = obj->GetElementStatus();

  GOMessageBox(
    wxString::Format(_("Status: %s"), status),
    obj->GetMidiTypeName() + _(" ") + obj->GetName(),
    wxOK);
}

void GOMidiListDialog::OnEdit(wxCommandEvent &event) {
  wxGridEvent listevent;
  OnObjectDoubleClick(listevent);
}
