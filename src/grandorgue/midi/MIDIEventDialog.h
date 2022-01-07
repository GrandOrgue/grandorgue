/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef MIDIEVENTDIALOG_H_
#define MIDIEVENTDIALOG_H_

#include "GOView.h"
#include <wx/propdlg.h>

class GOKeyReceiver;
class GOMidi;
class GOMidiListener;
class GOMidiReceiverBase;
class GOMidiSender;
class GOConfig;
class MIDIEventKeyDialog;
class MIDIEventRecvDialog;
class MIDIEventSendDialog;

class MIDIEventDialog : public wxPropertySheetDialog, public GOView
{
private:
	MIDIEventRecvDialog* m_recvPage;
	MIDIEventSendDialog* m_sendPage;
	MIDIEventSendDialog* m_sendDivisionPage;
	MIDIEventKeyDialog* m_keyPage;

	void DoApply();

	void OnApply(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

public:
	MIDIEventDialog (GODocumentBase* doc, wxWindow* parent, wxString title, GOConfig& settings, GOMidiReceiverBase* event, GOMidiSender* sender, GOKeyReceiver* key, GOMidiSender* division = NULL);
	~MIDIEventDialog();

	void RegisterMIDIListener(GOMidi* midi);

	DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
