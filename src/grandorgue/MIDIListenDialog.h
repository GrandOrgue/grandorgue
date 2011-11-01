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

class GOrgueMidiEvent;

class MIDIListenDialog : public wxDialog
{

DECLARE_CLASS(MIDIListenDialog)


public:

	typedef enum
	{
		LSTN_ENCLOSURE = 0,
		LSTN_MANUAL = 1,
		LSTN_SETTINGSDLG_MEMORY_OR_ORGAN = 2,
		LSTN_SETTINGSDLG_STOP_CHANGE = 3,
		LSTN_DRAWSTOP = 4,
		LSTN_NON_DRAWSTOP_BUTTON = 5
	} LISTEN_DIALOG_TYPE;

	typedef struct
	{
		const wxString name;
		int event;
	} LISTEN_DIALOG_SETUP_EVENT;

	typedef struct
	{
		unsigned count;
		const LISTEN_DIALOG_SETUP_EVENT *elements;
	} LISTEN_DIALOG_EVENTS;

private:

	static const LISTEN_DIALOG_EVENTS GetEventFromType(const LISTEN_DIALOG_TYPE type);
	LISTEN_DIALOG_TYPE m_type;
    int event;
	wxChoice* m_event;
	wxSpinCtrl *m_channel, *m_data;
	wxToggleButton* m_listen;

public:

	MIDIListenDialog
		(wxWindow* parent
		,wxString title
		,const LISTEN_DIALOG_TYPE type
		,const int event_id
		);

	~MIDIListenDialog();

	static unsigned GetEventChannel(unsigned what);
	static wxString GetEventTitle(int what, const LISTEN_DIALOG_TYPE type);
	static wxString GetEventChannelString(int what);

	int GetEvent();
	bool PutEvent(int what);
	void OnEvent(wxCommandEvent& event);
	void OnListenClick(wxCommandEvent& event);
	void OnMidiEvent(GOrgueMidiEvent& event);
	void OnHelp(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()


protected:

	enum {
		ID_EVENT = 200,
		ID_CHANNEL,
		ID_LISTEN,
	};

};

#endif /* MIDILISTENDIALOG_H_ */
