/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventDialog.h"

#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/msgdlg.h>

#include "config/GOConfig.h"
#include "midi/dialog-creator/GOMidiDialogListener.h"
#include "midi/objects/GOMidiObject.h"

#include "GOMidiEventKeyTab.h"
#include "GOMidiEventRecvTab.h"
#include "GOMidiEventSendTab.h"

enum {
  ID_BUTTON_TO_INITIAL = 400,
};

BEGIN_EVENT_TABLE(GOMidiEventDialog, GOTabbedDialog)
EVT_BUTTON(ID_BUTTON_TO_INITIAL, GOMidiEventDialog::OnButtonToInitial)
END_EVENT_TABLE()

GOMidiEventDialog::GOMidiEventDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  const wxString &title,
  GOConfig &settings,
  const wxString &dialogSelector,
  GOMidiObject *pMidiObject,
  bool mayBeAssignedToInitial,
  GOMidiReceiver *event,
  GOMidiSender *sender,
  GOMidiShortcutReceiver *key,
  GOMidiSender *division,
  GOMidiDialogListener *pDialogListener)
  : GOTabbedDialog(
    parent, "MidiEvent", title, settings.m_DialogSizes, dialogSelector),
    GOView(doc, this),
    r_config(settings),
    p_DialogListener(pDialogListener),
    p_object(pMidiObject),
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

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = GetButtonSizer();

  if (pButtonSizer) {
    pButtonSizer->InsertSpacer(2, 10);
    m_ToInitial = new wxButton(this, ID_BUTTON_TO_INITIAL, _("To initial"));
    pButtonSizer->Insert(
      3, m_ToInitial, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    pButtonSizer->InsertSpacer(3, 10);
    m_ToInitial->Enable(mayBeAssignedToInitial && pMidiObject);
  }

  LayoutDialog();
}

GOMidiEventDialog::GOMidiEventDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  const wxString &title,
  GOConfig &settings,
  const wxString &dialogSelector,
  GOMidiObject &midiObject,
  bool mayBeAssignedToInitial,
  GOMidiDialogListener *pDialogListener)
  : GOMidiEventDialog(
    doc,
    parent,
    title,
    settings,
    dialogSelector,
    &midiObject,
    mayBeAssignedToInitial,
    !midiObject.IsReadOnly() ? midiObject.GetMidiReceiver() : nullptr,
    midiObject.GetMidiSender(),
    !midiObject.IsReadOnly() ? midiObject.GetMidiShortcutReceiver() : nullptr,
    midiObject.GetDivisionSender(),
    pDialogListener) {}

GOMidiEventDialog::GOMidiEventDialog(
  wxWindow *parent,
  const wxString &title,
  GOConfig &settings,
  const wxString &dialogSelector,
  GOMidiReceiver *pReceiver)
  : GOMidiEventDialog(
    nullptr,
    parent,
    title,
    settings,
    dialogSelector,
    nullptr,
    false,
    pReceiver,
    nullptr,
    nullptr,
    nullptr,
    nullptr) {}

void GOMidiEventDialog::RegisterMIDIListener(GOMidi *midi) {
  if (m_recvPage)
    m_recvPage->RegisterMIDIListener(midi);
}

bool GOMidiEventDialog::TransferDataFromWindow() {
  bool res = GOTabbedDialog::TransferDataFromWindow();

  if (res && p_DialogListener)
    p_DialogListener->OnSettingsApplied();
  return res;
}

void GOMidiEventDialog::OnButtonToInitial(wxCommandEvent &e) {
  if (p_object) {
    TransferDataFromWindow();
    if (
      p_object->IsMidiConfigured()
      || wxMessageBox(
           _("No MIDI events are configured for this object.\n"
             "Are you sure to save the empty MIDI settings to initial?"),
           _("Empty MIDI settings"),
           wxCENTRE | wxYES_NO,
           this)
        == wxYES) {
      r_config.AssignToInitial(*p_object);
      r_config.Flush();
    }
  }
}
