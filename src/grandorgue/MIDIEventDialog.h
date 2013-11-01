/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MIDIEVENTDIALOG_H_
#define MIDIEVENTDIALOG_H_

#include <wx/wx.h>
#include <wx/propdlg.h>
#include "GrandOrgueDef.h"
#include "GOrgueDialogView.h"

class GOrgueMidi;
class GOrgueMidiReceiver;
class GOrgueMidiSender;
class GOrgueKeyReceiver;
class GOrgueSettings;
class MIDIEventRecvDialog;
class MIDIEventSendDialog;
class MIDIEventKeyDialog;

class MIDIEventDialog : public wxPropertySheetDialog, public GOrgueDialogView
{
private:
	MIDIEventRecvDialog* m_recvPage;
	MIDIEventSendDialog* m_sendPage;
	MIDIEventKeyDialog* m_keyPage;

	void DoApply();

	void OnApply(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

public:
	MIDIEventDialog (GOrgueDocument* doc, wxWindow* parent, wxString title, GOrgueSettings& settings, GOrgueMidiReceiver* event, GOrgueMidiSender* sender, GOrgueKeyReceiver* key);
	~MIDIEventDialog();

	void RegisterMIDIListener(GOrgueMidi* midi);

	DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
