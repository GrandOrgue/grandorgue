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

#include "GOrgueMidiFileReader.h"

#include "GOrgueMidiEvent.h"
#include "GOrgueMidiFile.h"

#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

GOrgueMidiFileReader::GOrgueMidiFileReader(GOrgueMidiMap& map) :
	m_Map(map),
	m_Data(0),
	m_DataLen(0),
	m_Tracks(0),
	m_Speed(0),
	m_Pos(0),
	m_TrackEnd(0),
	m_LastTime(0),
	m_PPQ(0),
	m_Tempo(0)
{
}

GOrgueMidiFileReader::~GOrgueMidiFileReader()
{
	if (m_Data)
		free(m_Data);
}

bool GOrgueMidiFileReader::Open(wxString filename)
{
	if (m_Data)
		free(m_Data);
	m_Data = NULL;

	wxFile file;
	if (!file.Open(filename, wxFile::read))
	{
		wxLogError(_("Failed to open %s"), filename.c_str());
		return false;
	}
	m_DataLen = file.Length();
	m_Data = (uint8_t*)malloc(m_DataLen);
	if (!m_Data)
	{
		wxLogError(_("Out of memory"));
		return false;
	}
	if (file.Read(m_Data, m_DataLen) != (ssize_t)m_DataLen)
	{
		wxLogError(_("Failed to read content of %s"), filename.c_str());
		return false;
	}
	if (m_DataLen < sizeof(MIDIHeaderChunk))
	{
		wxLogError(_("MIDI header missing"));
		return false;
	}
	m_Pos = sizeof(MIDIHeaderChunk);
	m_TrackEnd = 0;
	MIDIHeaderChunk* h = (MIDIHeaderChunk*)m_Data;
	if (memcmp(h->header.type, "MThd", sizeof(h->header.type)) || wxUINT32_SWAP_ON_LE(h->header.len) != 6)
	{
		wxLogError(_("Malformed MIDI header"));
		return false;
	}
	m_Tracks = wxUINT16_SWAP_ON_LE(h->tracks);
	switch (wxUINT16_SWAP_ON_LE(h->type))
	{
	case 0:
		if (m_Tracks != 1)
		{
			wxLogError(_("MIDI file type 0 only supports one track"));
			return false;
		}
		break;

	case 1:
		if (m_Tracks < 1)
		{
			wxLogError(_("MIDI file type 1 has not tracks"));
			return false;
		}
		break;

	case 2:
		wxLogError(_("MIDI file type 2 is not supported"));
		return false;

	default:
		wxLogError(_("Unkown MIDI file type %d"), wxUINT16_SWAP_ON_LE(h->type));
		return false;
	}
	m_Tempo = 0x7A120; // 120 BPM
	unsigned ppq = wxUINT16_SWAP_ON_LE(h->ppq);
	if (ppq & 0x8000)
	{
		unsigned frames = 1 + ((-ppq >> 8) & 0x7F);
		unsigned res = ppq & 0xFF;
		m_Speed = 1000.0 / frames / res;
		m_PPQ = 0;
	}
	else
	{
		m_PPQ = ppq;
		m_Speed = m_Tempo / 1000.0 / m_PPQ;
	}
	
	return true;
}


bool GOrgueMidiFileReader::StartTrack()
{
	do
	{
		if (m_Pos >= m_DataLen)
			return false;
		if (m_Pos + sizeof(MIDIFileHeader) > m_DataLen)
		{
			wxLogError(_("Incomplete chunk at offset %d"), m_Pos);
			m_Pos =+ sizeof(MIDIFileHeader);
			return false;
		}
		MIDIFileHeader* h = (MIDIFileHeader*)&m_Data[m_Pos];
		if (memcmp(h->type, "MTrk", sizeof(h->type)))
		{
			wxLogError(_("Not recognized MIDI chunk at offset %d"), m_Pos);
			m_Pos += sizeof(MIDIFileHeader) + wxUINT32_SWAP_ON_LE(h->len);
			if (m_Pos >= m_DataLen)
			{
				wxLogError(_("Incomplete chunk"));
				return false;
			}
		}
		else
		{
			m_TrackEnd = m_Pos + sizeof(MIDIFileHeader) + wxUINT32_SWAP_ON_LE(h->len);
			if (m_Pos >= m_DataLen)
			{
				wxLogError(_("Incomplete chunk"));
				return false;
			}
			if (!m_Tracks)
				wxLogError(_("Incorrect track count"));
			else
				m_Tracks--;
			m_Pos += sizeof(MIDIFileHeader);
			m_LastStatus = 0;
			m_LastTime = 0;
			m_CurrentSpeed = m_Speed;
			return true;
		}
	}
	while(true);
}

unsigned GOrgueMidiFileReader::DecodeTime()
{
	unsigned time = 0;
	if (m_Pos >= m_TrackEnd)
	{
		wxLogError(_("Incomplete timestamp at offset %d"), m_Pos);
		return 0;
	}
	uint8_t c = m_Data[m_Pos++];
	while((c & 0x80))
	{
		time = (time << 7) | (c & 0x7F);
		if (m_Pos >= m_TrackEnd)
		{
			wxLogError(_("Incomplete timestamp at offset %d"), m_Pos);
			return 0;
		}
		c = m_Data[m_Pos++];
	}
	time = (time << 7) | (c & 0x7F);
	return time;
}


bool GOrgueMidiFileReader::ReadEvent(GOrgueMidiEvent& e)
{
	do
	{
		std::vector<unsigned char> msg;
		if (m_Pos >= m_DataLen)
			return false;
		if (m_TrackEnd && m_Pos >= m_TrackEnd)
		{
			wxLogError(_("End of track marker missing at offset %d"), m_Pos);
			m_Pos = m_TrackEnd;
			m_TrackEnd = 0;
		}
		if (!m_TrackEnd)
			if (!StartTrack())
				return false;
		unsigned rel_time = DecodeTime();
		if (m_Pos >= m_TrackEnd)
			return false;
		unsigned len;
		if (m_Data[m_Pos] & 0x80)
		{
			msg.push_back(m_Data[m_Pos]);
			m_Pos++;
		}
		else if (m_LastStatus)
			msg.push_back(m_LastStatus);
		else
		{
			wxLogError(_("Status byte missing at offset %d"), m_Pos);
			return false;
		}
		m_LastStatus = 0;
		switch(msg[0] & 0xF0)
		{
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			m_LastStatus = msg[0];
			len = 2;
			break;

		case 0xC0:
		case 0xD0:
			m_LastStatus = msg[0];
			len = 1;
			break;

		case 0xF0:
			switch(msg[0])
			{
			case 0xF6:
			case 0xF8:
			case 0xF9:
			case 0xFA:
			case 0xFB:
			case 0xFC:
			case 0xFE:
				len = 0;
				break;

			case 0xF1:
			case 0xF3:
				len = 1;
				break;
			case 0xF2:
				len = 2;
				break;

			case 0xF0:
			case 0xF7:
				if (m_Pos + 1 > m_TrackEnd)
				{
					wxLogError(_("Incomplete MIDI message at %d"), m_Pos - 1);
					return false;
				}
				len = m_Data[m_Pos];
				m_Pos += 1;
				break;

			case 0xFF:
				if (m_Pos + 2 > m_TrackEnd)
				{
					wxLogError(_("Incomplete MIDI message at %d"), m_Pos - 1);
					return false;
				}
				msg.push_back(m_Data[m_Pos]);
				msg.push_back(m_Data[m_Pos + 1]);
				len = m_Data[m_Pos + 1];
				m_Pos += 2;
				break;

			default:
				wxLogError(_("Unknown MIDI message %02X at %d"), msg[0], m_Pos - 1);
				return false;
			}
			break;

		default:
			wxLogError(_("Unknown MIDI message %02X at %d"), msg[0], m_Pos - 1);
			return false;
		}
		if (m_Pos + len > m_TrackEnd)
		{
			wxLogError(_("Incomplete MIDI message at %d"), m_Pos - msg.size());
			return false;
		}
		for(unsigned i = 0; i < len; i++)
			msg.push_back(m_Data[m_Pos++]);
		m_LastTime += rel_time * m_Speed;

		if (msg[0] == 0xFF && msg[1] == 0x2F && msg[2] == 0x00)
		{
			if (m_Pos != m_TrackEnd)
				wxLogError(_("Events after end of track marker at %d"), m_Pos);

			m_Pos = m_TrackEnd;
			m_TrackEnd = 0;
			continue;
		}

		if (msg[0] == 0xFF && msg[1] == 0x51 && msg[2] == 0x03)
		{
			m_Tempo = (msg[3] << 16) | (msg[4] << 8) | (msg[5]);
			m_Speed = m_Tempo / 1000.0 / m_PPQ;
			continue;
		}
		
		e.FromMidi(msg, m_Map);
		e.SetTime(m_LastTime);
		if (e.GetMidiType() != MIDI_NONE)
			return true;
	}
	while(true);
}

bool GOrgueMidiFileReader::Close()
{
	if (m_Tracks)
		wxLogError(_("Some tracks are missing"));
	if (m_TrackEnd)
		wxLogError(_("Last track is missing the end marker"));
	if (m_Pos < m_TrackEnd)
	{
		wxLogError(_("Error decoding a track"));
		return false;
	}
	if (m_Pos < m_DataLen)
	{
		wxLogError(_("Garbage detected in the MIDI file"));
		return false;
	}
	return true;
}
