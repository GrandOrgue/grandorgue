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

#ifndef GORGUEMIDIRTINPORT_H
#define GORGUEMIDIRTINPORT_H

#include "GOrgueMidiInPort.h"
#include "RtMidi.h"

class GOrgueMidiRtInPort : public GOrgueMidiInPort
{
protected:
	RtMidiIn* m_port;

	static void MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData);

public:
	GOrgueMidiRtInPort(GOrgueMidi* midi, wxString prefix, wxString name, RtMidi::Api api);
	~GOrgueMidiRtInPort();

	bool Open(int channel_shift = 0);
	void Close();

	static void addMissingDevices(GOrgueMidi* midi, ptr_vector<GOrgueMidiInPort>& ports);
};

#endif
