/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiOutPort.h"

#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include <wx/intl.h>

GOrgueMidiOutPort::GOrgueMidiOutPort(GOrgueMidi* midi, wxString prefix, wxString name) :
	m_midi(midi),
	m_merger(),
	m_IsActive(false),
	m_Name(name),
	m_Prefix(prefix)
{
	m_ID = m_midi->GetMidiMap().GetDeviceByString(m_Name);
}

GOrgueMidiOutPort::~GOrgueMidiOutPort()
{
}

const wxString& GOrgueMidiOutPort::GetName()
{
	return m_Name;
}

unsigned GOrgueMidiOutPort::GetID()
{
	return m_ID;
}

bool GOrgueMidiOutPort::IsActive()
{
	return m_IsActive;
}

bool GOrgueMidiOutPort::Open()
{
	m_merger.Clear();
	return m_IsActive;
}

void GOrgueMidiOutPort::Send(const GOrgueMidiEvent& e)
{
	if (!IsActive())
		return;
	if (GetID() == e.GetDevice() || e.GetDevice() == 0)
	{
		GOrgueMidiEvent e1 = e;
		if (!m_merger.Process(e1))
			return;
		std::vector<std::vector<unsigned char>> msg;
		e1.ToMidi(msg, m_midi->GetMidiMap());
		for(unsigned i = 0; i < msg.size(); i++)
			SendData(msg[i]);
	}
}

const wxString GOrgueMidiOutPort::GetClientName()
{
	return wxT("GrandOrgue");
}

const wxString GOrgueMidiOutPort::GetPortName()
{
	return wxT("GrandOrgue Output");
}
