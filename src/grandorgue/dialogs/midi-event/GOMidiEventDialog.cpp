/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventDialog.h"

#include <wx/bookctrl.h>

#include "GOMidiEventKeyTab.h"
#include "GOMidiEventRecvTab.h"
#include "GOMidiEventSendTab.h"

GOMidiEventDialog::GOMidiEventDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  wxString title,
  GOConfig &settings,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOKeyReceiver *key,
  GOMidiSender *division)
  : GOTabbedDialog(parent, "MidiEvent", title),
    GOView(doc, this),
    m_recvPage(NULL),
    m_sendPage(NULL),
    m_sendDivisionPage(NULL),
    m_keyPage(NULL) {
  wxBookCtrlBase *notebook = GetBookCtrl();

  if (event) {
    m_recvPage = new GOMidiEventRecvTab(notebook, event, settings);
    AddTab(m_recvPage, "Receive", _("Receive"));
  }
  if (sender) {
    m_sendPage = new GOMidiEventSendTab(notebook, sender, m_recvPage, settings);
    AddTab(m_sendPage, "Send", _("Send"));
  }
  if (key) {
    m_keyPage = new GOMidiEventKeyTab(notebook, key);
    AddTab(m_keyPage, "Shortcut", _("Shortcut"));
  }
  if (division) {
    m_sendDivisionPage
      = new GOMidiEventSendTab(notebook, division, m_recvPage, settings);
    notebook->AddPage(m_sendDivisionPage, _("Send Division Output"));
    AddTab(m_sendDivisionPage, "SendDivisionOutput", _("Send Division Output"));
  }

  LayoutDialog();
}

void GOMidiEventDialog::RegisterMIDIListener(GOMidi *midi) {
  if (m_recvPage)
    m_recvPage->RegisterMIDIListener(midi);
}
