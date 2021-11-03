/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiInPort.h"

#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include <wx/intl.h>
#include <wx/stopwatch.h>

GOrgueMidiInPort::GOrgueMidiInPort(GOrgueMidi* midi, wxString prefix, wxString name) :
	m_midi(midi),
	m_IsActive(false),
	m_Name(name),
	m_Prefix(prefix),
	m_merger(),
	m_ChannelShift(0)
{
	m_ID = m_midi->GetMidiMap().GetDeviceByString(m_Name);
}

GOrgueMidiInPort::~GOrgueMidiInPort()
{
}

const wxString& GOrgueMidiInPort::GetName()
{
	return m_Name;
}

unsigned GOrgueMidiInPort::GetID()
{
	return m_ID;
}

bool GOrgueMidiInPort::IsActive()
{
	return m_IsActive;
}

void GOrgueMidiInPort::Receive(const std::vector<unsigned char> msg)
{
	if (!IsActive())
		return;

	GOrgueMidiEvent e;
	e.FromMidi(msg, m_midi->GetMidiMap());
	if (e.GetMidiType() == MIDI_NONE)
		return;
	e.SetDevice(GetID());
	e.SetTime(wxGetLocalTimeMillis());

	if (!m_merger.Process(e))
		return;

	/* Compat stuff */
	if (e.GetChannel() != -1)
		e.SetChannel(((e.GetChannel() - 1 + m_ChannelShift) & 0x0F) + 1);

	m_midi->Recv(e);
}

bool GOrgueMidiInPort::Open(int channel_shift)
{
	m_ChannelShift = channel_shift;
	m_merger.Clear();

	return m_IsActive;
}

const wxString GOrgueMidiInPort::GetClientName()
{
	return wxT("GrandOrgue");
}

const wxString GOrgueMidiInPort::GetPortName()
{
	return wxT("GrandOrgue Input");
}
