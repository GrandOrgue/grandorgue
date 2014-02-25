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

#include "GOrgueMidiRecorder.h"

#include "GOrgueMidiEvent.h"
#include "GOrgueMidiFile.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>
#include <wx/stopwatch.h>

GOrgueMidiRecorder::GOrgueMidiRecorder(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_NextChannel(0),
	m_NextNRPN(0),
	m_Mappings(),
	m_OutputDevice(0),
	m_file(),
	m_BufferPos(0),
	m_FileLength(0),
	m_Last(0)
{
	Clear();
}

GOrgueMidiRecorder::~GOrgueMidiRecorder()
{
	StopRecording();
}

void GOrgueMidiRecorder::SetOutputDevice(unsigned device_id)
{
	m_OutputDevice = device_id;
}

void GOrgueMidiRecorder::Clear()
{
	m_Mappings.clear();
	m_NextChannel = 1;
	m_NextNRPN = 0;

	GOrgueMidiEvent e;
	e.SetMidiType(MIDI_SYSEX_GO_CLEAR);
	e.SetDevice(m_OutputDevice);
	e.SetTime(wxGetLocalTimeMillis());
	if (m_OutputDevice)
		m_organfile->SendMidiMessage(e);
	if (IsRecording())
		WriteEvent(e);
}

void GOrgueMidiRecorder::SendMidiRecorderMessage(GOrgueMidiEvent& e)
{
	if (!m_OutputDevice && !IsRecording())
		return;
	if (e.GetDevice() >= m_Mappings.size())
		m_Mappings.resize(e.GetDevice() + 1);
	e.SetTime(wxGetLocalTimeMillis());

	if (e.GetDevice() != m_Mappings[e.GetDevice()].elementID)
	{
		if (e.GetMidiType() == MIDI_NRPN)
		{
			if (m_NextNRPN >= 1 << 18)
				return;
			m_Mappings[e.GetDevice()].elementID = e.GetDevice();
			m_Mappings[e.GetDevice()].channel = 1 + m_NextNRPN / (1 << 14);
			m_Mappings[e.GetDevice()].key = m_NextNRPN % (1 << 14);
			m_NextNRPN++;
		}
		else
		{
			if (m_NextChannel > 16)
				return;
			m_Mappings[e.GetDevice()].elementID = e.GetDevice();
			m_Mappings[e.GetDevice()].channel = m_NextChannel;
			m_Mappings[e.GetDevice()].key = 0;
			m_NextChannel++;
		}
		GOrgueMidiEvent e1;
		e1.SetTime(e.GetTime());
		e1.SetMidiType(MIDI_SYSEX_GO_SETUP);
		e1.SetDevice(m_OutputDevice);
		e1.SetKey(m_Mappings[e.GetDevice()].elementID);
		e1.SetChannel(m_Mappings[e.GetDevice()].channel);
		e1.SetValue(m_Mappings[e.GetDevice()].key);
		if (m_OutputDevice)
			m_organfile->SendMidiMessage(e1);
		WriteEvent(e1);
	}
	e.SetChannel(m_Mappings[e.GetDevice()].channel);
	if (e.GetMidiType() == MIDI_NRPN)
		e.SetKey(m_Mappings[e.GetDevice()].key);

	e.SetDevice(m_OutputDevice);
	if (m_OutputDevice)
		m_organfile->SendMidiMessage(e);
	WriteEvent(e);
}

void GOrgueMidiRecorder::Flush()
{
	if (!m_BufferPos)
		return;
	m_file.Write(m_Buffer, m_BufferPos);
	m_FileLength += m_BufferPos;
	m_BufferPos = 0;
}


void GOrgueMidiRecorder::Ensure(unsigned length)
{
	if (m_BufferPos + length < sizeof(m_Buffer))
		return;
	Flush();
}

void GOrgueMidiRecorder::Write(const void* data, unsigned len)
{
	if (len >= sizeof(m_Buffer))
	{
		Flush();
		m_file.Write(data, len);
		m_FileLength += len;
		return;
	}
	Ensure(len);
	memcpy(m_Buffer + m_BufferPos, data, len);
	m_BufferPos += len;
}


void GOrgueMidiRecorder::EncodeLength(unsigned len)
{
	char buf[16];
	unsigned l = sizeof(buf) - 1;
	buf[l] = len & 0x7F;
	len = len >> 7;
	while(len)
	{
		l--;
		buf[l] = 0x80 | (len & 0x7F);
		len = len >> 7;
	}
	Write(buf + l, sizeof(buf) - l);
}

void GOrgueMidiRecorder::WriteEvent(GOrgueMidiEvent& e)
{
	if (!IsRecording())
		return;
	std::vector<std::vector<unsigned char>> msg;
	e.ToMidi(msg, m_organfile->GetSettings().GetMidiMap());
	for(unsigned i = 0; i < msg.size(); i++)
	{
		EncodeLength((e.GetTime() - m_Last).GetValue());
		if (msg[i][0] == 0xF0)
		{
			Write(&msg[i][0], 1);
			EncodeLength(msg[i].size() - 1);
			Write(&msg[i][1], msg[i].size() - 1);
		}
		else
			Write(&msg[i][0], msg[i].size());
		m_Last = e.GetTime();
	}
}

bool GOrgueMidiRecorder::IsRecording()
{
	return m_file.IsOpened();
}

void GOrgueMidiRecorder::StopRecording()
{
	if (!IsRecording())
		return;
	const unsigned char end[4] = { 0x01, 0xFF, 0x2F, 0x00 };
	Write(end, sizeof(end));
	Flush();
	m_FileLength -= sizeof(MIDIHeaderChunk) + sizeof(MIDIFileHeader);
	MIDIFileHeader h = { { 'M', 'T', 'r', 'k' }, (uint32_t)wxUINT32_SWAP_ON_LE(m_FileLength) };
	m_file.Seek(sizeof(MIDIHeaderChunk));
	m_file.Write(&h, sizeof(h));
	m_file.Close();
}

void GOrgueMidiRecorder::StartRecording(wxString filename)
{
	MIDIHeaderChunk h = { { { 'M', 'T', 'h', 'd' }, (uint32_t)wxUINT32_SWAP_ON_LE(6) }, 
			      (uint16_t)wxUINT16_SWAP_ON_LE(0),(uint16_t)wxUINT16_SWAP_ON_LE(1), (uint16_t)wxUINT16_SWAP_ON_LE(0xE728) };
	MIDIFileHeader t = { { 'M', 'T', 'r', 'k' }, 0 };

	StopRecording();

        m_file.Create(filename, true);
        if (!m_file.IsOpened())
	{
		wxLogError(_("Unable to open file for writing"));
		return;
	}
	
	m_FileLength = 0;
	m_BufferPos = 0;
	Write(&h, sizeof(h));
	Write(&t, sizeof(t));

	wxString s = m_organfile->GetChurchName();
	wxCharBuffer b = s.ToAscii();
	unsigned len = s.length();
	unsigned char th[] = { 0x00, 0xFF, 0x04 };
	Write(&th, sizeof(th));
	EncodeLength(len);
	Write(b.data(), len);

	m_Last = wxGetLocalTimeMillis();
	m_organfile->PrepareRecording();
}
