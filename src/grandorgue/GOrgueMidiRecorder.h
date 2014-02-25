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

#ifndef GORGUEMIDIRECORDER_H
#define GORGUEMIDIRECORDER_H

#include "GOrgueTime.h"

#include <wx/file.h>
#include <wx/string.h>
#include <vector>

class GOrgueMidiEvent;
class GrandOrgueFile;

class GOrgueMidiRecorder
{
	typedef struct
	{
		unsigned elementID;
		unsigned channel;
		unsigned key;
	} midi_map;
private:
	GrandOrgueFile* m_organfile;
	unsigned m_NextChannel;
	unsigned m_NextNRPN;
	std::vector<midi_map> m_Mappings;
	unsigned m_OutputDevice;
	wxFile m_file;
	char m_Buffer[2000];
	unsigned m_BufferPos;
	unsigned m_FileLength;
	GOTime m_Last;

	void Ensure(unsigned length);
	void Flush();
	void EncodeLength(unsigned len);
	void Write(const void* data, unsigned len);
	void WriteEvent(GOrgueMidiEvent& e);

public:
	GOrgueMidiRecorder(GrandOrgueFile* organfile);
	~GOrgueMidiRecorder();

	void SetOutputDevice(unsigned device_id);

	void SendMidiRecorderMessage(GOrgueMidiEvent& e);

	void Clear();

	void StartRecording(wxString filename);
	bool IsRecording();
	void StopRecording();
};

#endif
