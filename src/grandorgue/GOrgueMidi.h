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

#ifndef GORGUEMIDI_H
#define GORGUEMIDI_H

#include "ptrvector.h"
#include <wx/event.h>
#include <map>

class GOrgueMidiEvent;
class GOrgueMidiListener;
class GOrgueSettings;
class GrandOrgueFile;
class RtMidiIn;
class RtMidiOut;

class GOrgueMidi : public wxEvtHandler
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
		bool rpn;
		unsigned rpn_lsb[16];
		unsigned rpn_msb[16];
		unsigned nrpn_lsb[16];
		unsigned nrpn_msb[16];
		GOrgueMidi* midi;
	} MIDI_IN_DEVICE;

	typedef struct {
		RtMidiOut* midi_out;
		wxString name;
		bool active;
		int rtmidi_port_no;
	} MIDI_OUT_DEVICE;

	GOrgueSettings& m_Settings;
	std::map<wxString, int> m_midi_in_device_map;
	ptr_vector<MIDI_IN_DEVICE> m_midi_in_devices;
	std::map<wxString, int> m_midi_out_device_map;
	ptr_vector<MIDI_OUT_DEVICE> m_midi_out_devices;
	int m_transpose;
	std::vector<GOrgueMidiListener*> m_Listeners;

	void ProcessMessage(std::vector<unsigned char>& msg, MIDI_IN_DEVICE* device);

	static void MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData);
	void OnMidiEvent(GOrgueMidiEvent& event);

public:

	GOrgueMidi(GOrgueSettings& settings);
	~GOrgueMidi();

	void Open();
	void UpdateDevices();

	void Send(GOrgueMidiEvent& e);

	std::map<wxString, int>& GetInDevices();
	std::map<wxString, int>& GetOutDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);
	void Register(GOrgueMidiListener* listener);
	void Unregister(GOrgueMidiListener* listener);

	DECLARE_EVENT_TABLE()
};

#endif /* GORGUEMIDI_H */
