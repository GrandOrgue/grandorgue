/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventDialog.h"

#include <wx/bookctrl.h>

#include <config/GOConfig.h>

#include "GOMidiEventKeyTab.h"
#include "GOMidiEventRecvTab.h"
#include "GOMidiEventSendTab.h"

GOMidiEventDialog::GOMidiEventDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  const wxString &title,
  GOConfig &settings,
  const wxString &dialogSelector,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOMidiShortcutReceiver *key,
  GOMidiSender *division)
  : GOTabbedDialog(
    parent, "MidiEvent", title, settings.m_DialogSizes, dialogSelector),
    GOView(doc, this),
    m_recvPage(NULL),
    m_sendPage(NULL),
    m_sendDivisionPage(NULL),
    m_keyPage(NULL) {
  wxBookCtrlBase *notebook = GetBookCtrl();

  if (event) {
    m_recvPage = new GOMidiEventRecvTab(notebook, event, settings);
    m_recvPage->SetModificationListener(this);
    AddTab(m_recvPage, "Receive", _("Receive"));
  }
  if (sender) {
    m_sendPage
      = new GOMidiEventSendTab(this, _("Send"), sender, m_recvPage, settings);
    m_sendPage->SetModificationListener(this);
    AddTab(m_sendPage);
  }
  if (key) {
    m_keyPage = new GOMidiEventKeyTab(notebook, key);
    AddTab(m_keyPage, "Shortcut", _("Shortcut"));
  }
  if (division) {
    m_sendDivisionPage = new GOMidiEventSendTab(
      this, _("Send Division Output"), division, m_recvPage, settings);
    AddTab(m_sendDivisionPage);
  }

  LayoutDialog();
}

void GOMidiEventDialog::RegisterMIDIListener(GOMidi *midi) {
  if (m_recvPage)
    m_recvPage->RegisterMIDIListener(midi);
}
