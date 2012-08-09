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

#ifndef GORGUEMIDI_H
#define GORGUEMIDI_H

#include <map>
#include <wx/wx.h>
#include "GrandOrgueDef.h"
#include "ptrvector.h"

class RtMidiIn;

class GOrgueSettings;
class GrandOrgueFile;

class GOrgueMidi
{

private:

	typedef struct {
		RtMidiIn* midi_in;
		wxString name;
		bool active;
		int channel_shift;
		int rtmidi_port_no;
		unsigned bank_lsb[16];
		unsigned bank_msb[16];
		GOrgueMidi* midi;
	} MIDI_IN_DEVICE;

	GOrgueSettings& m_Settings;
	std::map<wxString, int> m_midi_in_device_map;
	ptr_vector<MIDI_IN_DEVICE> m_midi_in_devices;
	int m_transpose;
	bool m_listening;
	wxEvtHandler* m_listen_evthandler;

	void ProcessMessage(std::vector<unsigned char>& msg, MIDI_IN_DEVICE* device);

	static void MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData);

public:

	GOrgueMidi(GOrgueSettings& settings);
	~GOrgueMidi();

	void Open();
	void UpdateDevices();

	bool HasListener();
	void SetListener(wxEvtHandler* event_handler);
	std::map<wxString, int>& GetInDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);
};

#endif /* GORGUEMIDI_H */
