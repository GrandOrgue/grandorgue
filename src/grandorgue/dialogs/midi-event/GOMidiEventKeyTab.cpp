/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
    m_key.GetType() == KEY_RECV_ENCLOSURE ? _("&Plus-Shortcut:")
                                          : _("&Shortcut:"));
  m_keyselect = new wxChoice(this, ID_KEY_SELECT);
  sizer->Add(m_keyselect, 1, wxEXPAND);

  sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
  m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
  sizer->Add(m_listen, 0, wxTOP, 5);

  topSizer->Add(sizer, 0, wxALL | wxEXPAND, 6);

  if (m_key.GetType() == KEY_RECV_ENCLOSURE) {
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
  const GOShortcutKey *keys = GetShortcutKeys();
  unsigned count = GetShortcutKeyCount();
  select->Append(_("None"), (void *)0);
  select->SetSelection(0);
  for (unsigned i = 0; i < count; i++) {
    select->Append(wxGetTranslation(keys[i].name), (void *)&keys[i]);
    if (keys[i].key_code == shortcut)
      select->SetSelection(i + 1);
  }
}

bool GOMidiEventKeyTab::TransferDataFromWindow() {
  const GOShortcutKey *key = (const GOShortcutKey *)m_keyselect->GetClientData(
    m_keyselect->GetSelection());
  if (!key)
    m_key.SetShortcut(0);
  else
    m_key.SetShortcut(key->key_code);
  if (m_keyminusselect) {
    key = (const GOShortcutKey *)m_keyminusselect->GetClientData(
      m_keyminusselect->GetSelection());
    if (!key)
      m_key.SetMinusKey(0);
    else
      m_key.SetMinusKey(key->key_code);
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
  unsigned code = WXKtoVK(event.GetKeyCode());
  if (code) {
    wxChoice *select = m_keyselect;
    if (m_minuslisten && m_minuslisten->GetValue())
      select = m_keyminusselect;
    for (unsigned i = 0; i < select->GetCount(); i++) {
      const GOShortcutKey *key
        = (const GOShortcutKey *)select->GetClientData(i);
      if (key && key->key_code == code)
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
