/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsTemperaments.h"

#include <wx/button.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stopwatch.h>

#include "GOTime.h"
#include "config/GOConfig.h"
#include "go_limits.h"
#include "temperaments/GOTemperamentUser.h"

BEGIN_EVENT_TABLE(GOSettingsTemperaments, wxPanel)
EVT_GRID_CMD_SELECT_CELL(ID_LIST, GOSettingsTemperaments::OnListSelected)
EVT_BUTTON(ID_ADD, GOSettingsTemperaments::OnAdd)
EVT_BUTTON(ID_DEL, GOSettingsTemperaments::OnDel)
END_EVENT_TABLE()

GOSettingsTemperaments::GOSettingsTemperaments(
  GOConfig &settings, wxWindow *parent)
  : wxPanel(parent, wxID_ANY),
    m_Temperaments(settings.GetTemperaments()),
    m_Ptrs() {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_List = new wxGrid(this, ID_LIST, wxDefaultPosition, wxSize(400, 200));
  m_List->CreateGrid(0, 14);
  topSizer->Add(m_List, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_Add = new wxButton(this, ID_ADD, _("&Add"));
  m_Del = new wxButton(this, ID_DEL, _("&Delete"));
  buttonSizer->Add(m_Add, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL, 5);

  m_List->SetColLabelValue(0, _("Group"));
  m_List->SetColLabelValue(1, _("Name"));
  m_List->SetColLabelValue(2, _("c"));
  m_List->SetColLabelValue(3, _("c#"));
  m_List->SetColLabelValue(4, _("d"));
  m_List->SetColLabelValue(5, _("d#"));
  m_List->SetColLabelValue(6, _("e"));
  m_List->SetColLabelValue(7, _("f"));
  m_List->SetColLabelValue(8, _("f#"));
  m_List->SetColLabelValue(9, _("g"));
  m_List->SetColLabelValue(10, _("g#"));
  m_List->SetColLabelValue(11, _("a"));
  m_List->SetColLabelValue(12, _("a#"));
  m_List->SetColLabelValue(13, _("b"));

  m_List->SetColMinimalWidth(0, 75);
  m_List->SetColMinimalWidth(1, 150);

  for (unsigned i = 0; i < 12; i++) {
    m_List->SetColFormatFloat(2 + i, 8, 3);
    m_List->SetColMinimalWidth(2 + i, 50);
  }

  m_List->SetDefaultEditor(new wxGridCellFloatEditor(8, 3));

  for (unsigned i = 0; i < m_Temperaments.GetUserTemperaments().size(); i++) {
    GOTemperamentUser *t = m_Temperaments.GetUserTemperaments()[i];

    unsigned row = m_List->GetNumberRows();
    m_Ptrs.push_back(t);
    m_List->AppendRows(1);
    m_List->SetCellValue(row, 0, t->GetGroup());
    m_List->SetCellEditor(row, 0, new wxGridCellTextEditor());
    m_List->SetCellValue(row, 1, t->GetTitle());
    m_List->SetCellEditor(row, 1, new wxGridCellTextEditor());
    for (unsigned i = 0; i < 12; i++)
      m_List->SetCellValue(
        row, 2 + i, wxString::Format(wxT("%.03f"), t->GetNoteOffset(i)));
  }

  m_List->EnableEditing(true);
  m_List->AutoSizeColumns(false);
  m_List->SetRowLabelSize(0);

  topSizer->AddSpacer(5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);

  Update();
}

void GOSettingsTemperaments::OnAdd(wxCommandEvent &event) {
  unsigned row = m_List->GetNumberRows();
  m_Ptrs.push_back(NULL);
  m_List->AppendRows(1);
  m_List->SetCellValue(row, 0, _("User"));
  m_List->SetCellEditor(row, 0, new wxGridCellTextEditor());
  m_List->SetCellValue(row, 1, _("User Temperament"));
  m_List->SetCellEditor(row, 1, new wxGridCellTextEditor());
  for (unsigned i = 0; i < 12; i++)
    m_List->SetCellValue(row, 2 + i, wxString::Format(wxT("%.03f"), 0.0));
  Update();
}

void GOSettingsTemperaments::OnDel(wxCommandEvent &event) {
  unsigned row = m_List->GetGridCursorRow();
  m_List->DeleteRows(row, 1);
  m_Ptrs.erase(m_Ptrs.begin() + row);
  Update();
}

void GOSettingsTemperaments::OnListSelected(wxGridEvent &event) {
  event.Skip();
  Update();
}

void GOSettingsTemperaments::Update() {
  if (m_Ptrs.size() > 0 && m_List->GetGridCursorRow() >= 0)
    m_Del->Enable();
  else
    m_Del->Disable();
  if (m_Ptrs.size() < MAX_TEMPERAMENTS)
    m_Add->Enable();
  else
    m_Add->Disable();
}

bool GOSettingsTemperaments::TransferDataFromWindow() {
  ptr_vector<GOTemperamentUser> &list = m_Temperaments.GetUserTemperaments();
  for (unsigned i = 0; i < list.size(); i++) {
    bool found = false;
    for (unsigned j = 0; j < m_Ptrs.size(); j++)
      if (m_Ptrs[j] == list[i])
        found = true;

    if (!found)
      delete list[i];
    list[i] = 0;
  }
  list.clear();
  long time = wxGetUTCTime();
  for (unsigned i = 0; i < m_Ptrs.size(); i++) {
    if (!m_Ptrs[i])
      m_Ptrs[i] = new GOTemperamentUser(
        wxString::Format(wxT("UserTemperament%d-%d"), time, i),
        m_List->GetCellValue(i, 1),
        m_List->GetCellValue(i, 0),
        m_List->GetCellValue(i, 0));

    m_Ptrs[i]->SetTitle(m_List->GetCellValue(i, 1));
    m_Ptrs[i]->SetGroup(m_List->GetCellValue(i, 0));
    for (unsigned j = 0; j < 12; j++)
      m_Ptrs[i]->SetNoteOffset(j, wxAtof(m_List->GetCellValue(i, 2 + j)));

    list.push_back(m_Ptrs[i]);
  }
  return true;
}
