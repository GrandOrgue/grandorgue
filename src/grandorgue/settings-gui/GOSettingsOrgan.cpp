/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsOrgan.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "GOOrgan.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "midi/MIDIEventDialog.h"

BEGIN_EVENT_TABLE(GOSettingsOrgan, wxPanel)
EVT_LIST_ITEM_SELECTED(ID_ORGANS, GOSettingsOrgan::OnOrganSelected)
EVT_BUTTON(ID_UP, GOSettingsOrgan::OnUp)
EVT_BUTTON(ID_DOWN, GOSettingsOrgan::OnDown)
EVT_BUTTON(ID_TOP, GOSettingsOrgan::OnTop)
EVT_BUTTON(ID_DEL, GOSettingsOrgan::OnDel)
EVT_BUTTON(ID_PROPERTIES, GOSettingsOrgan::OnProperties)
END_EVENT_TABLE()

GOSettingsOrgan::GOSettingsOrgan(
  GOConfig &settings, GOMidi &midi, wxWindow *parent)
  : wxPanel(parent, wxID_ANY), m_config(settings), m_midi(midi) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_Organs = new wxListView(
    this,
    ID_ORGANS,
    wxDefaultPosition,
    wxSize(100, 200),
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  m_Organs->InsertColumn(0, _("Church"));
  m_Organs->InsertColumn(1, _("Builder"));
  m_Organs->InsertColumn(2, _("Recording"));
  m_Organs->InsertColumn(3, _("MIDI"));
  m_Organs->InsertColumn(4, _("Organ package"));
  m_Organs->InsertColumn(5, _("ODF Path"));
  topSizer->Add(m_Organs, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
  m_Down = new wxButton(this, ID_DOWN, _("&Down"));
  m_Up = new wxButton(this, ID_UP, _("&Up"));
  m_Top = new wxButton(this, ID_TOP, _("&Top"));
  m_Del = new wxButton(this, ID_DEL, _("&Delete"));
  buttonSizer->Add(m_Down, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_Up, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_Top, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_Properties, 0, wxALIGN_LEFT | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL, 5);

  for (unsigned i = 0; i < m_config.GetOrganList().size(); i++) {
    GOOrgan *o = m_config.GetOrganList()[i];
    wxString title = o->GetChurchName();
    if (!o->IsUsable(m_config))
      title = _("MISSING - ") + title;
    m_Organs->InsertItem(i, title);
    m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
    m_Organs->SetItem(i, 1, o->GetOrganBuilder());
    m_Organs->SetItem(i, 2, o->GetRecordingDetail());
    m_Organs->SetItem(
      i, 3, o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
    m_Organs->SetItem(i, 5, o->GetODFPath());
    if (o->GetArchiveID() != wxEmptyString) {
      const GOArchiveFile *a = m_config.GetArchiveByID(o->GetArchiveID());
      m_Organs->SetItem(i, 4, a ? a->GetName() : o->GetArchiveID());
    }
  }
  if (m_Organs->GetItemCount()) {
    m_Organs->SetColumnWidth(0, 150);
    m_Organs->SetColumnWidth(1, 150);
    m_Organs->SetColumnWidth(2, 250);
    m_Organs->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
    m_Organs->SetColumnWidth(4, wxLIST_AUTOSIZE);
    m_Organs->SetColumnWidth(5, wxLIST_AUTOSIZE);
  }
  m_Up->Disable();
  m_Down->Disable();
  m_Top->Disable();
  m_Del->Disable();
  m_Properties->Disable();

  topSizer->AddSpacer(5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);
}

void GOSettingsOrgan::OnOrganSelected(wxListEvent &event) {
  long index = m_Organs->GetFirstSelected();
  m_Del->Enable();
  m_Properties->Enable();
  if (index <= 0) {
    m_Top->Disable();
    m_Up->Disable();
  } else {
    m_Top->Enable();
    m_Up->Enable();
  }
  if (
    m_Organs->GetItemCount() > 1 && index >= 0
    && index + 1 < m_Organs->GetItemCount())
    m_Down->Enable();
  else
    m_Down->Disable();
}

void GOSettingsOrgan::MoveOrgan(long from, long to) {
  wxListItem item;
  item.SetId(from);
  item.SetMask(-1);
  m_Organs->GetItem(item);
  item.SetId(to);
  m_Organs->InsertItem(item);
  if (to < from)
    from++;
  for (unsigned i = 0; i < 6; i++) {
    item.SetId(from);
    item.SetColumn(i);
    m_Organs->GetItem(item);
    item.SetId(to);
    m_Organs->SetItem(item);
  }
  m_Organs->SetItemPtrData(to, m_Organs->GetItemData(from));
  m_Organs->DeleteItem(from);
  m_Organs->Select(to);
}

void GOSettingsOrgan::OnUp(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() - 1);
}

void GOSettingsOrgan::OnDown(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() + 1);
}

void GOSettingsOrgan::OnTop(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), 0);
}

void GOSettingsOrgan::OnDel(wxCommandEvent &event) {
  if (
    wxMessageBox(
      wxString::Format(
        _("Do you want to remove %s?"),
        m_Organs->GetItemText(m_Organs->GetFirstSelected()).c_str()),
      _("Organs"),
      wxYES_NO | wxICON_EXCLAMATION,
      this)
    == wxYES) {
    m_Organs->DeleteItem(m_Organs->GetFirstSelected());
    m_Up->Disable();
    m_Down->Disable();
    m_Top->Disable();
    m_Del->Disable();
    m_Properties->Disable();
  }
}

void GOSettingsOrgan::OnProperties(wxCommandEvent &event) {
  GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(m_Organs->GetFirstSelected());
  MIDIEventDialog dlg(
    NULL,
    this,
    wxString::Format(
      _("MIDI settings for organ %s"), o->GetChurchName().c_str()),
    m_config,
    &o->GetMIDIReceiver(),
    NULL,
    NULL);
  dlg.RegisterMIDIListener(&m_midi);
  dlg.ShowModal();
  m_Organs->SetItem(
    m_Organs->GetFirstSelected(),
    3,
    o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
}

std::vector<const GOOrgan *> GOSettingsOrgan::GetOrgans() {
  std::vector<const GOOrgan *> list;
  for (long j = 0; j < m_Organs->GetItemCount(); j++)
    list.push_back((const GOOrgan *)m_Organs->GetItemData(j));
  return list;
}

void GOSettingsOrgan::Save() {
  ptr_vector<GOOrgan> &list = m_config.GetOrganList();
  for (unsigned i = 0; i < list.size(); i++) {
    bool found = false;
    for (long j = 0; j < m_Organs->GetItemCount(); j++)
      if (m_Organs->GetItemData(j) == (wxUIntPtr)list[i])
        found = true;
    if (!found)
      delete list[i];
    list[i] = 0;
  }
  list.clear();
  for (long i = 0; i < m_Organs->GetItemCount(); i++)
    list.push_back((GOOrgan *)m_Organs->GetItemData(i));
}
