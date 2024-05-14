/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiListDialog.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include "midi/GOMidiConfigurator.h"

#include "GOEvent.h"

BEGIN_EVENT_TABLE(GOMidiListDialog, GOSimpleDialog)
EVT_LIST_ITEM_SELECTED(ID_LIST, GOMidiListDialog::OnObjectClick)
EVT_LIST_ITEM_ACTIVATED(ID_LIST, GOMidiListDialog::OnObjectDoubleClick)
EVT_BUTTON(ID_STATUS, GOMidiListDialog::OnStatus)
EVT_BUTTON(ID_EDIT, GOMidiListDialog::OnEdit)
EVT_COMMAND_RANGE(
  ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON, GOMidiListDialog::OnButton)
END_EVENT_TABLE()

GOMidiListDialog::GOMidiListDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  GODialogSizeSet &dialogSizes,
  const std::vector<GOMidiConfigurator *> &midi_elements)
  : GOSimpleDialog(
    parent,
    wxT("MIDI Objects"),
    _("MIDI Objects"),
    dialogSizes,
    wxEmptyString,
    0,
    wxOK | wxHELP),
    GOView(doc, this) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_Objects = new wxListView(
    this,
    ID_LIST,
    wxDefaultPosition,
    wxSize(300, 600),
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  m_Objects->InsertColumn(0, _("Type"));
  m_Objects->InsertColumn(1, _("Element"));
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

  for (unsigned i = 0; i < midi_elements.size(); i++) {
    GOMidiConfigurator *obj = midi_elements[i];

    m_Objects->InsertItem(i, obj->GetMidiType());
    m_Objects->SetItemPtrData(i, (wxUIntPtr)obj);
    m_Objects->SetItem(i, 1, obj->GetMidiName());
  }

  m_Objects->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_Objects->SetColumnWidth(1, wxLIST_AUTOSIZE);

  topSizer->AddSpacer(5);
  LayoutWithInnerSizer(topSizer);
}

GOMidiListDialog::~GOMidiListDialog() {}

void GOMidiListDialog::OnButton(wxCommandEvent &event) {
  GOMidiConfigurator *obj = (GOMidiConfigurator *)m_Objects->GetItemData(
    m_Objects->GetFirstSelected());
  obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}

void GOMidiListDialog::OnStatus(wxCommandEvent &event) {
  GOMidiConfigurator *obj = (GOMidiConfigurator *)m_Objects->GetItemData(
    m_Objects->GetFirstSelected());
  wxString status = obj->GetElementStatus();
  GOMessageBox(
    wxString::Format(_("Status: %s"), status),
    obj->GetMidiType() + _(" ") + obj->GetMidiName(),
    wxOK);
}

void GOMidiListDialog::OnObjectClick(wxListEvent &event) {
  m_Edit->Enable();
  m_Status->Enable();
  GOMidiConfigurator *obj = (GOMidiConfigurator *)m_Objects->GetItemData(
    m_Objects->GetFirstSelected());
  std::vector<wxString> actions = obj->GetElementActions();
  for (unsigned i = 0; i < m_Buttons.size(); i++)
    if (i < actions.size()) {
      m_Buttons[i]->SetLabel(actions[i]);
      m_Buttons[i]->Show();
    } else
      m_Buttons[i]->Hide();
  Layout();
}

void GOMidiListDialog::OnObjectDoubleClick(wxListEvent &event) {
  GOMidiConfigurator *obj = (GOMidiConfigurator *)m_Objects->GetItemData(
    m_Objects->GetFirstSelected());
  obj->ShowConfigDialog();
}

void GOMidiListDialog::OnEdit(wxCommandEvent &event) {
  wxListEvent listevent;
  OnObjectDoubleClick(listevent);
}
