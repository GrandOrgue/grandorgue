/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueMidiPlayer.h"
#include "GOrgueMidiRecorder.h"
#include "ptrvector.h"
#include <wx/event.h>

class GOrgueMidiEvent;
class GOrgueMidiInPort;
class GOrgueMidiListener;
class GOrgueMidiMap;
class GOrgueMidiOutPort;
class GOrgueSettings;
class GrandOrgueFile;
class wxMidiEvent;

class GOrgueMidi : public wxEvtHandler
{
private:
	GOrgueSettings& m_Settings;
	ptr_vector<GOrgueMidiInPort> m_midi_in_devices;
	ptr_vector<GOrgueMidiOutPort> m_midi_out_devices;
	int m_transpose;
	std::vector<GOrgueMidiListener*> m_Listeners;
	GOrgueMidiRecorder m_MidiRecorder;
	GOrgueMidiPlayer m_MidiPlayer;

	void OnMidiEvent(wxMidiEvent& event);

public:

	GOrgueMidi(GOrgueSettings& settings);
	~GOrgueMidi();

	void Open();
	void UpdateDevices();

	void Recv(const GOrgueMidiEvent& e);
	void Send(const GOrgueMidiEvent& e);

	std::vector<wxString> GetInDevices();
	std::vector<wxString> GetOutDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);
	void Register(GOrgueMidiListener* listener);
	void Unregister(GOrgueMidiListener* listener);

	GOrgueMidiMap& GetMidiMap();
	GOrgueMidiRecorder& GetMidiRecorder();
	GOrgueMidiPlayer& GetMidiPlayer();

	DECLARE_EVENT_TABLE()
};

#endif /* GORGUEMIDI_H */
