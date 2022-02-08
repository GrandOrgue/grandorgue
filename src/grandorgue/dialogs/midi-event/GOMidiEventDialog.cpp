/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventDialog.h"

#include <wx/bookctrl.h>
#include <wx/msgdlg.h>

#include "GOMidiEventKeyTab.h"
#include "GOMidiEventRecvTab.h"
#include "GOMidiEventSendTab.h"

BEGIN_EVENT_TABLE(MIDIEventDialog, wxPropertySheetDialog)
EVT_BUTTON(wxID_APPLY, MIDIEventDialog::OnApply)
EVT_BUTTON(wxID_OK, MIDIEventDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, MIDIEventDialog::OnCancel)
END_EVENT_TABLE()

MIDIEventDialog::MIDIEventDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  wxString title,
  GOConfig &settings,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOKeyReceiver *key,
  GOMidiSender *division)
  : wxPropertySheetDialog(
    parent,
    wxID_ANY,
    title,
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    GOView(doc, this),
    m_recvPage(NULL),
    m_sendPage(NULL),
    m_sendDivisionPage(NULL),
    m_keyPage(NULL) {
  CreateButtons(wxOK | wxCANCEL);

  wxBookCtrlBase *notebook = GetBookCtrl();

  if (event) {
    m_recvPage = new MIDIEventRecvDialog(notebook, event, settings);
    notebook->AddPage(m_recvPage, _("Receive"));
  }
  if (sender) {
    m_sendPage
      = new MIDIEventSendDialog(notebook, sender, m_recvPage, settings);
    notebook->AddPage(m_sendPage, _("Send"));
  }
  if (key) {
    m_keyPage = new MIDIEventKeyDialog(notebook, key);
    notebook->AddPage(m_keyPage, _("Shortcut"));
  }
  if (division) {
    m_sendDivisionPage
      = new MIDIEventSendDialog(notebook, division, m_recvPage, settings);
    notebook->AddPage(m_sendDivisionPage, _("Send Division Output"));
  }

  LayoutDialog();
}

MIDIEventDialog::~MIDIEventDialog() {}

void MIDIEventDialog::RegisterMIDIListener(GOMidi *midi) {
  if (m_recvPage)
    m_recvPage->RegisterMIDIListener(midi);
}

bool MIDIEventDialog::Validate() {
  wxWindow *notValidTab = NULL;
  wxString errMsg;

  if (!notValidTab && m_sendPage && !m_sendPage->Validate(errMsg))
    notValidTab = m_sendPage;
  if (
    !notValidTab && m_sendDivisionPage && !m_sendDivisionPage->Validate(errMsg))
    notValidTab = m_sendPage;

  // Display the tab that has problems
  if (notValidTab) {
    wxBookCtrlBase *const notebook = GetBookCtrl();
    const int notValidPageId = notebook->FindPage(notValidTab);

    if (notValidPageId != wxNOT_FOUND)
      notebook->SetSelection(notValidPageId);
    if (!errMsg.IsEmpty())
      wxMessageBox(
        errMsg, _("Invalid MIDI event"), wxOK | wxCENTRE | wxICON_ERROR);
  }

  return !notValidTab;
}

void MIDIEventDialog::OnApply(wxCommandEvent &event) {
  if (Validate())
    DoApply();
}

void MIDIEventDialog::OnOK(wxCommandEvent &event) {
  if (Validate()) {
    DoApply();
    if (HasDocument())
      Destroy();
    else
      EndModal(wxID_OK);
  }
}

void MIDIEventDialog::OnCancel(wxCommandEvent &event) {
  if (HasDocument())
    Destroy();
  else
    EndModal(wxID_CANCEL);
}

void MIDIEventDialog::DoApply() {
  if (m_recvPage)
    m_recvPage->DoApply();
  if (m_sendPage)
    m_sendPage->DoApply();
  if (m_sendDivisionPage)
    m_sendDivisionPage->DoApply();
  if (m_keyPage)
    m_keyPage->DoApply();
}
