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

#ifndef GORGUEMIDI_H
#define GORGUEMIDI_H

#include <vector>
#include <map>
#include <wx/wx.h>
#include "MIDIListenDialog.h"

class RtMidiIn;

class wxConfigBase;
class GrandOrgueFile;

#define NB_MIDI_EVENTS (16)

class GOrgueMidi
{

private:

	typedef struct {
		RtMidiIn* midi_in;
		wxString name;
		bool active;
		int id;
		unsigned bank_lsb[16];
		unsigned bank_msb[16];
	} MIDI_DEVICE;

	wxConfigBase *m_global_config;
	std::map<wxString, int> m_midi_device_map;
	std::vector<MIDI_DEVICE> m_midi_devices;
	int m_transpose;
	bool m_listening;
	wxEvtHandler* m_listen_evthandler;
	std::map<long, wxString> m_organ_midi_events;
	GrandOrgueFile* m_organfile;

	int m_midi_events[NB_MIDI_EVENTS];

	void ProcessMessage
		(const bool active
		,std::vector<unsigned char>& msg
		,int which
		);

public:

	GOrgueMidi();
	~GOrgueMidi();

	void Open();
	void SetOrganFile(GrandOrgueFile* organfile);
	void ProcessMessages(const bool audio_active);

	bool HasListener();
	void SetListener(wxEvtHandler* event_handler);
	void UpdateOrganMIDI();
	std::map<wxString, int>& GetDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);

	int GetMidiEventByChannel(int channel);
	int GetStopMidiEvent();
	int GetManualMidiEvent(int manual_nb);

	static unsigned NbMidiEvents();
	static wxString GetMidiEventTitle(const unsigned idx);
	static wxString GetMidiEventUserTitle(const unsigned idx);
	static MIDIListenDialog::LISTEN_DIALOG_TYPE GetMidiEventListenDialogType(const unsigned idx);


};

#endif /* GORGUEMIDI_H */
