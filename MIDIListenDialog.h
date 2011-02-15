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

#ifndef MIDILISTENDIALOG_H_
#define MIDILISTENDIALOG_H_

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

class MIDIListenDialog : public wxDialog
{
DECLARE_CLASS(MIDIListenDialog)
public:
	MIDIListenDialog(wxWindow* parent, wxString title, int what, int type);
	~MIDIListenDialog();

	int GetEvent();
	bool PutEvent(int what);
	void OnEvent(wxCommandEvent& event);
	void OnListenClick(wxCommandEvent& event);
	void OnListenMIDI(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);

	int event;
	wxChoice* m_event;
	wxSpinCtrl *m_channel, *m_data;

	DECLARE_EVENT_TABLE()

protected:
	enum {
		ID_EVENT = 200,
		ID_CHANNEL,
		ID_LISTEN,
	};

    int type;
	wxToggleButton* m_listen;

};

wxString GetEventTitle(int what, int type);
int GetEventChannel(int what);
wxString GetEventChannelString(int what);

#endif /* MIDILISTENDIALOG_H_ */
