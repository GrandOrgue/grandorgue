/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventKeyTab.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

#include "GOKeyConvert.h"

BEGIN_EVENT_TABLE(GOMidiEventKeyTab, wxPanel)
EVT_TOGGLEBUTTON(ID_LISTEN, GOMidiEventKeyTab::OnListenClick)
EVT_TOGGLEBUTTON(ID_LISTEN_MINUS, GOMidiEventKeyTab::OnMinusListenClick)
END_EVENT_TABLE()

GOMidiEventKeyTab::GOMidiEventKeyTab(
  wxWindow *parent, GOMidiShortcutReceiver *event)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
    m_original(event),
    m_key(*event),
    m_keyminusselect(NULL),
    m_minuslisten(NULL) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *sizer = new wxStaticBoxSizer(
    wxVERTICAL,
    this,
    m_key.GetType() == GOMidiShortcutPattern::KEY_RECV_ENCLOSURE
      ? _("&Plus-Shortcut:")
      : _("&Shortcut:"));
  m_keyselect = new wxChoice(this, ID_KEY_SELECT);
  sizer->Add(m_keyselect, 1, wxEXPAND);

  sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
  m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
  sizer->Add(m_listen, 0, wxTOP, 5);

  topSizer->Add(sizer, 0, wxALL | wxEXPAND, 6);

  if (m_key.GetType() == GOMidiShortcutPattern::KEY_RECV_ENCLOSURE) {
    sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("&Minus-Shortcut:"));
    m_keyminusselect = new wxChoice(this, ID_MINUS_KEY_SELECT);
    sizer->Add(m_keyminusselect, 1, wxEXPAND);

    sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
    m_minuslisten
      = new wxToggleButton(this, ID_LISTEN_MINUS, _("&Listen for Event"));
    sizer->Add(m_minuslisten, 0, wxTOP, 5);

    topSizer->Add(sizer, 0, wxALL | wxEXPAND, 6);
  }

  SetSizer(topSizer);

  FillKeylist(m_keyselect, m_key.GetShortcut());
  if (m_keyminusselect)
    FillKeylist(m_keyminusselect, m_key.GetMinusKey());

  topSizer->Fit(this);
}

GOMidiEventKeyTab::~GOMidiEventKeyTab() {}

void GOMidiEventKeyTab::FillKeylist(wxChoice *select, unsigned shortcut) {
  const auto &keys = GOKeyConvert::SHORTCUTS.m_entries;
  select->Append(_("None"), (void *)nullptr);
  select->SetSelection(0);
  for (unsigned l = keys.size(), i = 0; i < l; i++) {
    const GOConfigEnum::Entry &key = keys[i];

    select->Append(wxGetTranslation(key.name), (void *)&key);
    if (key.value == (int)shortcut)
      select->SetSelection(i + 1);
  }
}

bool GOMidiEventKeyTab::TransferDataFromWindow() {
  const GOConfigEnum::Entry *pKey
    = (const GOConfigEnum::Entry *)m_keyselect->GetClientData(
      m_keyselect->GetSelection());

  m_key.SetShortcut(pKey ? (unsigned)pKey->value : 0);
  if (m_keyminusselect) {
    pKey = (const GOConfigEnum::Entry *)m_keyminusselect->GetClientData(
      m_keyminusselect->GetSelection());
    m_key.SetMinusKey(pKey ? (unsigned)pKey->value : 0);
  }
  if (m_original->RenewFrom(m_key))
    GOModificationProxy::OnIsModifiedChanged(true);
  return true;
}

void GOMidiEventKeyTab::Listen(bool enable) {
  if (enable) {
    this->SetCursor(wxCursor(wxCURSOR_WAIT));
    m_listen->GetEventHandler()->Connect(
      wxEVT_KEY_DOWN,
      wxKeyEventHandler(GOMidiEventKeyTab::OnKeyDown),
      NULL,
      this);
    if (m_minuslisten)
      m_minuslisten->GetEventHandler()->Connect(
        wxEVT_KEY_DOWN,
        wxKeyEventHandler(GOMidiEventKeyTab::OnKeyDown),
        NULL,
        this);
  } else {
    m_listen->GetEventHandler()->Disconnect(
      wxEVT_KEY_DOWN,
      wxKeyEventHandler(GOMidiEventKeyTab::OnKeyDown),
      NULL,
      this);
    if (m_minuslisten)
      m_minuslisten->GetEventHandler()->Disconnect(
        wxEVT_KEY_DOWN,
        wxKeyEventHandler(GOMidiEventKeyTab::OnKeyDown),
        NULL,
        this);
    this->SetCursor(wxCursor(wxCURSOR_ARROW));
  }
}

void GOMidiEventKeyTab::OnKeyDown(wxKeyEvent &event) {
  unsigned code = GOKeyConvert::wXKtoVK(event.GetKeyCode());
  if (code) {
    wxChoice *select = (m_minuslisten && m_minuslisten->GetValue())
      ? m_keyminusselect
      : m_keyselect;

    for (unsigned i = 0; i < select->GetCount(); i++) {
      const GOConfigEnum::Entry *pKey
        = (const GOConfigEnum::Entry *)select->GetClientData(i);
      if (pKey && pKey->value == (int)code)
        select->SetSelection(i);
    }

    m_listen->SetValue(false);
    if (m_minuslisten)
      m_minuslisten->SetValue(false);
    Listen(false);
  } else
    event.Skip();
}

void GOMidiEventKeyTab::OnListenClick(wxCommandEvent &event) {
  if (m_minuslisten && m_minuslisten->GetValue()) {
    Listen(false);
    m_minuslisten->SetValue(false);
  }
  if (m_listen->GetValue()) {
    Listen(true);
  } else {
    Listen(false);
  }
}

void GOMidiEventKeyTab::OnMinusListenClick(wxCommandEvent &event) {
  if (m_listen->GetValue()) {
    Listen(false);
    m_listen->SetValue(false);
  }
  if (m_minuslisten->GetValue()) {
    Listen(true);
  } else {
    Listen(false);
  }
}
