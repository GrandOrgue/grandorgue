/*
 * MIDIListenDialog.h
 *
 *  Created on: 15/02/2011
 *      Author: nappleton
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
