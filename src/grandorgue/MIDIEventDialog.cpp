/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include <wx/bookctrl.h>
#include "MIDIEventDialog.h"
#include "MIDIEventRecvDialog.h"
#include "MIDIEventSendDialog.h"

IMPLEMENT_CLASS(MIDIEventDialog, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(MIDIEventDialog, wxPropertySheetDialog)
END_EVENT_TABLE()


MIDIEventDialog::MIDIEventDialog (wxWindow* parent, wxString title, const GOrgueMidiReceiver* event, const GOrgueMidiSender* sender):
	wxPropertySheetDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,330)),
	m_recvPage(NULL),
	m_sendPage(NULL)
{
	CreateButtons(wxOK | wxCANCEL);

	wxBookCtrlBase* notebook = GetBookCtrl();

	if (event)
	{
		m_recvPage = new MIDIEventRecvDialog(notebook, *event);
		notebook->AddPage(m_recvPage,  _("Receive"));
	}
	if (sender)
	{
		m_sendPage = new MIDIEventSendDialog(notebook, *sender);
		notebook->AddPage(m_sendPage,  _("Send"));
	}
}

MIDIEventDialog::~MIDIEventDialog()
{
}

const GOrgueMidiReceiver& MIDIEventDialog::GetResult()
{
	return m_recvPage->GetResult();
}

const GOrgueMidiSender& MIDIEventDialog::GetSender()
{
	return m_sendPage->GetResult();
}

