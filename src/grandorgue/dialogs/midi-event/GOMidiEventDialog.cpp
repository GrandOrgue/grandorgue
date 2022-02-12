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

BEGIN_EVENT_TABLE(GOMidiEventDialog, wxPropertySheetDialog)
EVT_BUTTON(wxID_APPLY, GOMidiEventDialog::OnApply)
EVT_BUTTON(wxID_OK, GOMidiEventDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, GOMidiEventDialog::OnCancel)
END_EVENT_TABLE()

GOMidiEventDialog::GOMidiEventDialog(
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
    m_recvPage = new GOMidiEventRecvTab(notebook, event, settings);
    notebook->AddPage(m_recvPage, _("Receive"));
  }
  if (sender) {
    m_sendPage = new GOMidiEventSendTab(notebook, sender, m_recvPage, settings);
    notebook->AddPage(m_sendPage, _("Send"));
  }
  if (key) {
    m_keyPage = new GOMidiEventKeyTab(notebook, key);
    notebook->AddPage(m_keyPage, _("Shortcut"));
  }
  if (division) {
    m_sendDivisionPage
      = new GOMidiEventSendTab(notebook, division, m_recvPage, settings);
    notebook->AddPage(m_sendDivisionPage, _("Send Division Output"));
  }

  LayoutDialog();
}

GOMidiEventDialog::~GOMidiEventDialog() {}

void GOMidiEventDialog::RegisterMIDIListener(GOMidi *midi) {
  if (m_recvPage)
    m_recvPage->RegisterMIDIListener(midi);
}

bool GOMidiEventDialog::Validate() {
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

void GOMidiEventDialog::OnApply(wxCommandEvent &event) {
  if (Validate())
    DoApply();
}

void GOMidiEventDialog::OnOK(wxCommandEvent &event) {
  if (Validate()) {
    DoApply();
    if (HasDocument())
      Destroy();
    else
      EndModal(wxID_OK);
  }
}

void GOMidiEventDialog::OnCancel(wxCommandEvent &event) {
  if (HasDocument())
    Destroy();
  else
    EndModal(wxID_CANCEL);
}

void GOMidiEventDialog::DoApply() {
  if (m_recvPage)
    m_recvPage->DoApply();
  if (m_sendPage)
    m_sendPage->DoApply();
  if (m_sendDivisionPage)
    m_sendDivisionPage->DoApply();
  if (m_keyPage)
    m_keyPage->DoApply();
}
