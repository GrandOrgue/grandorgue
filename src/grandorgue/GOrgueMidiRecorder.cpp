/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiFile.h"
#include "GOrguePath.h"
#include "GOrgueSetterButton.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/stopwatch.h>

enum {
	ID_MIDI_RECORDER_RECORD = 0,
	ID_MIDI_RECORDER_STOP,
	ID_MIDI_RECORDER_RECORD_RENAME,
};

const struct ElementListEntry GOrgueMidiRecorder::m_element_types[] = {
	{ wxT("MidiRecorderRecord"), ID_MIDI_RECORDER_RECORD, false, true },
	{ wxT("MidiRecorderStop"), ID_MIDI_RECORDER_STOP, false, true },
	{ wxT("MidiRecorderRecordRename"), ID_MIDI_RECORDER_RECORD_RENAME, false, true },
	{ wxT(""), -1, false, false },
};

const struct ElementListEntry* GOrgueMidiRecorder::GetButtonList()
{
	return m_element_types;
}

GOrgueMidiRecorder::GOrgueMidiRecorder(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Map(organfile->GetSettings().GetMidiMap()),
	m_NextChannel(0),
	m_NextNRPN(0),
	m_Mappings(),
	m_Preconfig(),
	m_OutputDevice(0),
	m_file(),
	m_Filename(),
	m_DoRename(false),
	m_BufferPos(0),
	m_FileLength(0),
	m_Last(0)
{
	CreateButtons(m_organfile);
	Clear();
}

GOrgueMidiRecorder::~GOrgueMidiRecorder()
{
	StopRecording();
}

void GOrgueMidiRecorder::Load(GOrgueConfigReader& cfg)
{
	m_button[ID_MIDI_RECORDER_RECORD]->Init(cfg, wxT("MidiRecorderRecord"), _("REC"));
	m_button[ID_MIDI_RECORDER_STOP]->Init(cfg, wxT("MidiRecorderStop"), _("STOP"));
	m_button[ID_MIDI_RECORDER_RECORD_RENAME]->Init(cfg, wxT("MidiRecorderRecordRename"), _("REC File"));
}

void GOrgueMidiRecorder::ButtonChanged(int id)
{
	switch(id)
	{
	case ID_MIDI_RECORDER_STOP:
		StopRecording();
		break;

	case ID_MIDI_RECORDER_RECORD:
		StartRecording(false);
		break;

	case ID_MIDI_RECORDER_RECORD_RENAME:
		StartRecording(true);
		break;
	}
}

void GOrgueMidiRecorder::SetOutputDevice(const wxString& device_id)
{
	m_OutputDevice = m_Map.GetDeviceByString(device_id);
}

void GOrgueMidiRecorder::SendEvent(GOrgueMidiEvent& e)
{
	e.SetDevice(m_OutputDevice);
	if (m_OutputDevice)
		m_organfile->SendMidiMessage(e);
	if (IsRecording())
		WriteEvent(e);
}

void GOrgueMidiRecorder::Clear()
{
	m_Mappings.clear();
	m_Preconfig.clear();
	m_NextChannel = 1;
	m_NextNRPN = 0;

	GOrgueMidiEvent e;
	e.SetMidiType(MIDI_SYSEX_GO_CLEAR);
	e.SetTime(wxGetLocalTimeMillis());
	SendEvent(e);
}

void GOrgueMidiRecorder::PreconfigureMapping(const wxString& element, bool isNRPN)
{
	PreconfigureMapping(element, isNRPN, element);
}

void GOrgueMidiRecorder::PreconfigureMapping(const wxString& element, bool isNRPN, const wxString& reference)
{
	unsigned id = m_Map.GetElementByString(element);
	unsigned ref = m_Map.GetElementByString(reference);
	for(unsigned i = 0; i < m_Preconfig.size(); i++)
		if (m_Preconfig[i].elementID == ref)
		{
			GOrgueMidiEvent e1;
			e1.SetTime(wxGetLocalTimeMillis());
			e1.SetMidiType(MIDI_SYSEX_GO_SETUP);
			e1.SetKey(id);
			e1.SetChannel(m_Preconfig[i].channel);
			e1.SetValue(m_Preconfig[i].key);
			SendEvent(e1);

			midi_map m = m_Preconfig[i];
			m.elementID = ref;
			if (ref >= m_Mappings.size())
				m_Mappings.resize(ref + 1);
			m_Mappings[ref] = m;
			return;
		}

	midi_map m;
	if (isNRPN)
	{
		if (m_NextNRPN >= 1 << 18)
			return;
		m.elementID = ref;
		m.channel = 1 + m_NextNRPN / (1 << 14);
		m.key = m_NextNRPN % (1 << 14);
		m_NextNRPN++;
	}
	else
	{
		if (m_NextChannel > 16)
			return;
		m.elementID = ref;
		m.channel = m_NextChannel;
		m.key = 0;
		m_NextChannel++;
	}
	m_Preconfig.push_back(m);

	GOrgueMidiEvent e1;
	e1.SetTime(wxGetLocalTimeMillis());
	e1.SetMidiType(MIDI_SYSEX_GO_SETUP);
	e1.SetKey(id);
	e1.SetChannel(m.channel);
	e1.SetValue(m.key);
	SendEvent(e1);

	if (ref >= m_Mappings.size())
		m_Mappings.resize(ref + 1);
	m_Mappings[ref] = m;
}

void GOrgueMidiRecorder::SetSamplesetId(unsigned id1, unsigned id2)
{
	GOrgueMidiEvent e1;
	e1.SetTime(wxGetLocalTimeMillis());
	e1.SetMidiType(MIDI_SYSEX_GO_SAMPLESET);
	e1.SetKey(id1);
	e1.SetValue(id2);
	SendEvent(e1);

	m_Mappings.clear();
}


bool GOrgueMidiRecorder::SetupMapping(unsigned element, bool isNRPN)
{
	if (element >= m_Mappings.size())
		m_Mappings.resize(element + 1);

	if (element != m_Mappings[element].elementID)
	{
		if (isNRPN)
		{
			if (m_NextNRPN >= 1 << 18)
				return false;
			m_Mappings[element].elementID = element;
			m_Mappings[element].channel = 1 + m_NextNRPN / (1 << 14);
			m_Mappings[element].key = m_NextNRPN % (1 << 14);
			m_NextNRPN++;
		}
		else
		{
			if (m_NextChannel > 16)
				return false;
			m_Mappings[element].elementID = element;
			m_Mappings[element].channel = m_NextChannel;
			m_Mappings[element].key = 0;
			m_NextChannel++;
		}
		GOrgueMidiEvent e1;
		e1.SetTime(wxGetLocalTimeMillis());
		e1.SetMidiType(MIDI_SYSEX_GO_SETUP);
		e1.SetKey(m_Mappings[element].elementID);
		e1.SetChannel(m_Mappings[element].channel);
		e1.SetValue(m_Mappings[element].key);
		SendEvent(e1);
	}
	return true;
}

void GOrgueMidiRecorder::SendMidiRecorderMessage(GOrgueMidiEvent& e)
{
	if (!m_OutputDevice && !IsRecording())
		return;
	if (!SetupMapping(e.GetDevice(), e.GetMidiType() == MIDI_NRPN))
		return;

	e.SetTime(wxGetLocalTimeMillis());
	e.SetChannel(m_Mappings[e.GetDevice()].channel);
	if (e.GetMidiType() == MIDI_NRPN)
		e.SetKey(m_Mappings[e.GetDevice()].key);

	SendEvent(e);
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
	e.ToMidi(msg, m_Map);
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
	m_button[ID_MIDI_RECORDER_RECORD]->Display(false);
	m_button[ID_MIDI_RECORDER_RECORD_RENAME]->Display(false);
	if (!IsRecording())
		return;
	const unsigned char end[4] = { 0x01, 0xFF, 0x2F, 0x00 };
	Write(end, sizeof(end));
	Flush();
	m_FileLength -= sizeof(MIDIHeaderChunk) + sizeof(MIDIFileHeader);
	MIDIFileHeader h = { { 'M', 'T', 'r', 'k' }, (uint32_t)wxUINT32_SWAP_ON_LE(m_FileLength) };
	m_file.Seek(sizeof(MIDIHeaderChunk));
	m_file.Write(&h, sizeof(h));
	m_file.Flush();
	m_file.Close();
	if (!m_DoRename)
	{
		wxFileName name = m_Filename;
		GOSyncDirectory(name.GetPath());
	}
	else
		GOAskRenameFile(m_Filename, m_organfile->GetSettings().MidiRecorderPath(),_("MIDI files (*.mid)|*.mid"));
}

void GOrgueMidiRecorder::StartRecording(bool rename)
{
	MIDIHeaderChunk h = { { { 'M', 'T', 'h', 'd' }, (uint32_t)wxUINT32_SWAP_ON_LE(6) }, 
			      (uint16_t)wxUINT16_SWAP_ON_LE(0),(uint16_t)wxUINT16_SWAP_ON_LE(1), (uint16_t)wxUINT16_SWAP_ON_LE(0xE728) };
	MIDIFileHeader t = { { 'M', 'T', 'r', 'k' }, 0 };

	StopRecording();
	if (!m_organfile)
		return;

	m_Filename = m_organfile->GetSettings().MidiRecorderPath() + wxFileName::GetPathSeparator() + wxDateTime::UNow().Format(_("%Y-%m-%d:%H:%M:%S.%l.mid"));
	m_DoRename = rename;

        m_file.Create(m_Filename, true);
        if (!m_file.IsOpened())
	{
		wxLogError(_("Unable to open file %s for writing"), m_Filename.c_str());
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
	if (m_DoRename)
		m_button[ID_MIDI_RECORDER_RECORD_RENAME]->Display(true);
	else
		m_button[ID_MIDI_RECORDER_RECORD]->Display(true);
	m_organfile->PrepareRecording();
}

GOrgueEnclosure* GOrgueMidiRecorder::GetEnclosure(const wxString& name, bool is_panel)
{
	return NULL;
}

GOrgueLabel* GOrgueMidiRecorder::GetLabel(const wxString& name, bool is_panel)
{
	if (is_panel)
		return NULL;

	return NULL;
}
