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

#include "GOrgueMidiOutPort.h"

#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include <wx/intl.h>

GOrgueMidiOutPort::GOrgueMidiOutPort(GOrgueMidi* midi, wxString prefix, wxString name) :
	m_midi(midi),
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
	return m_IsActive;
}

void GOrgueMidiOutPort::Send(const GOrgueMidiEvent& e)
{
	if (!IsActive())
		return;
	if (GetID() == e.GetDevice() || e.GetDevice() == 0)
	{
		std::vector<std::vector<unsigned char>> msg;
		e.ToMidi(msg, m_midi->GetMidiMap());
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
	return _("GrandOrgue Output");
}
