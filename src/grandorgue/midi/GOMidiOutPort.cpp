/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiOutPort.h"

#include "GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiMap.h"
#include <wx/intl.h>

GOMidiOutPort::GOMidiOutPort(GOMidi* midi, wxString prefix, wxString name) :
	m_midi(midi),
	m_merger(),
	m_IsActive(false),
	m_Name(name),
	m_Prefix(prefix)
{
	m_ID = m_midi->GetMidiMap().GetDeviceByString(m_Name);
}

GOMidiOutPort::~GOMidiOutPort()
{
}

const wxString& GOMidiOutPort::GetName()
{
	return m_Name;
}

unsigned GOMidiOutPort::GetID()
{
	return m_ID;
}

bool GOMidiOutPort::IsActive()
{
	return m_IsActive;
}

bool GOMidiOutPort::Open()
{
	m_merger.Clear();
	return m_IsActive;
}

void GOMidiOutPort::Send(const GOMidiEvent& e)
{
	if (!IsActive())
		return;
	if (GetID() == e.GetDevice() || e.GetDevice() == 0)
	{
		GOMidiEvent e1 = e;
		if (!m_merger.Process(e1))
			return;
		std::vector<std::vector<unsigned char>> msg;
		e1.ToMidi(msg, m_midi->GetMidiMap());
		for(unsigned i = 0; i < msg.size(); i++)
			SendData(msg[i]);
	}
}

const wxString GOMidiOutPort::GetClientName()
{
	return wxT("GrandOrgue");
}

const wxString GOMidiOutPort::GetPortName()
{
	return wxT("GrandOrgue Output");
}
