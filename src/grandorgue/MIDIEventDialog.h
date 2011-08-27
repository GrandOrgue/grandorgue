/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef MIDIEVENTDIALOG_H_
#define MIDIEVENTDIALOG_H_

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>
#include "GOrgueMidiReceiver.h"

class MIDIEventDialog : public wxDialog
{

DECLARE_CLASS(MIDIEventDialog)

private:

	GOrgueMidiReceiver m_midi;
	wxChoice *m_eventno, *m_eventtype, *m_channel, *m_device;
	wxSpinCtrl *m_data;
	wxToggleButton* m_listen;
	wxButton* m_new, *m_delete;
	int m_current;

	void StoreEvent();
	void LoadEvent();

public:

	MIDIEventDialog (wxWindow* parent, wxString title, const GOrgueMidiReceiver& event);
	~MIDIEventDialog();

	const GOrgueMidiReceiver& GetResult();

	DECLARE_EVENT_TABLE()

	void OnListenClick(wxCommandEvent& event);
	void OnNewClick(wxCommandEvent& event);
	void OnDeleteClick(wxCommandEvent& event);
	void OnEventChange(wxCommandEvent& event);
	void OnMidiEvent(GOrgueMidiEvent& event);

protected:

	enum {
		ID_EVENT_NO = 200,
		ID_EVENT_NEW,
		ID_EVENT_DELETE,
		ID_DEVICE,
		ID_EVENT,
		ID_CHANNEL,
		ID_DATA,
		ID_LISTEN,
	};

};

#endif /* MIDIEVENTDIALOG_H_ */
