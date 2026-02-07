/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTDIALOG_H
#define GOMIDIEVENTDIALOG_H

#include <wx/propdlg.h>

#include "document-base/GOView.h"
#include "gui/dialogs/common/GOTabbedDialog.h"
#include "modification/GOModificationProxy.h"

class wxButton;

class GOConfig;
class GOMidiSystem;
class GOMidiDialogListener;
class GOMidiEventKeyTab;
class GOMidiEventRecvTab;
class GOMidiEventSendTab;
class GOMidiListener;
class GOMidiObject;
class GOMidiReceiver;
class GOMidiSender;
class GOMidiShortcutReceiver;

class GOMidiEventDialog : public GOTabbedDialog,
                          public GOView,
                          public GOModificationProxy {
private:
  GOConfig &r_config;
  GOMidiDialogListener *p_DialogListener;

  GOMidiObject *p_object;
  GOMidiEventRecvTab *m_recvPage;
  GOMidiEventSendTab *m_sendPage;
  GOMidiEventSendTab *m_sendDivisionPage;
  GOMidiEventKeyTab *m_keyPage;

  wxButton *m_ToInitial;

  GOMidiEventDialog(
    GODocumentBase *doc,
    /*
      if doc != NULL then the dialog is auto destroyed when closed
      if doc == NULL then the caller should call ShowModal() and then should
      call Destroy() if needed
    */
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
    GOMidiDialogListener *pDialogListener);

public:
  GOMidiEventDialog(
    GODocumentBase *doc,
    /*
      if doc != NULL then the dialog is auto destroyed when closed
      if doc == NULL then the caller should call ShowModal() and then should
      call Destroy() if needed
    */
    wxWindow *parent,
    const wxString &title,
    GOConfig &settings,
    const wxString &dialogSelector,
    GOMidiObject &midiObject,
    bool mayBeAssignedToInitial,
    GOMidiDialogListener *pDialogListener = nullptr);

  GOMidiEventDialog(
    wxWindow *parent,
    const wxString &title,
    GOConfig &settings,
    const wxString &dialogSelector,
    GOMidiReceiver *pReceiver);

  void RegisterMIDIListener(GOMidiSystem *midi);

private:
  void OnButtonToInitial(wxCommandEvent &e);
  bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
