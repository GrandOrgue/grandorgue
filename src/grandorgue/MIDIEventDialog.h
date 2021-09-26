/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef MIDIEVENTDIALOG_H_
#define MIDIEVENTDIALOG_H_

#include "GOrgueView.h"
#include <wx/propdlg.h>

class GOrgueKeyReceiver;
class GOrgueMidi;
class GOrgueMidiListener;
class GOrgueMidiReceiverBase;
class GOrgueMidiSender;
class GOrgueSettings;
class MIDIEventKeyDialog;
class MIDIEventRecvDialog;
class MIDIEventSendDialog;

class MIDIEventDialog : public wxPropertySheetDialog, public GOrgueView
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
	MIDIEventDialog (GOrgueDocumentBase* doc, wxWindow* parent, wxString title, GOrgueSettings& settings, GOrgueMidiReceiverBase* event, GOrgueMidiSender* sender, GOrgueKeyReceiver* key, GOrgueMidiSender* division = NULL);
	~MIDIEventDialog();

	void RegisterMIDIListener(GOrgueMidi* midi);

	DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
