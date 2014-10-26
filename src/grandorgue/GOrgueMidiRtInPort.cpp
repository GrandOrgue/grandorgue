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

#include "GOrgueMidiRtInPort.h"
#include "GOrgueRtHelpers.h"
#include <wx/intl.h>
#include <wx/log.h>

GOrgueMidiRtInPort::GOrgueMidiRtInPort(GOrgueMidi* midi, wxString prefix, wxString name, RtMidi::Api api) :
	GOrgueMidiInPort(midi, prefix, name),
	m_port(NULL)
{
	try
	{
		m_port = new RtMidiIn(api, (const char*)GetClientName().fn_str());
		m_port->ignoreTypes(false);
		m_port->setCallback(&MIDICallback, this);
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}

GOrgueMidiRtInPort::~GOrgueMidiRtInPort()
{
	Close();
	try
	{
		if (m_port)
			delete m_port;
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}

bool GOrgueMidiRtInPort::Open(int channel_shift)
{
	Close();
	if (!m_port)
		return false;
	try
	{
		for (unsigned i = 0; i <  m_port->getPortCount(); i++)
		{
			if (m_Name == m_Prefix + wxString::FromAscii(m_port->getPortName(i).c_str()))
			{
				m_port->openPort(i, (const char*)GetPortName().fn_str());
				m_IsActive = true;
				break;
			}
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
	return GOrgueMidiInPort::Open(channel_shift);
}

void GOrgueMidiRtInPort::Close()
{
	if (!m_IsActive)
		return;
	try
	{
		m_port->closePort();
		m_IsActive = false;
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}

void GOrgueMidiRtInPort::MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData)
{
	GOrgueMidiRtInPort* port = (GOrgueMidiRtInPort*)userData;
	port->Receive(*msg);
}

void GOrgueMidiRtInPort::addMissingDevices(GOrgueMidi* midi, ptr_vector<GOrgueMidiInPort>& ports)
{
	try
	{
		std::vector<RtMidi::Api> apis;
		RtMidi::getCompiledApi(apis);

		for(unsigned i = 0; i < apis.size(); i++)
		{
			try
			{
				RtMidiIn midi_dev(apis[i]);
				for (unsigned j = 0; j < midi_dev.getPortCount(); j++)
				{
					wxString name = GOrgueRtHelpers::GetMidiApiPrefix(apis[i]) + wxString::FromAscii(midi_dev.getPortName(j).c_str());
					bool found = false;
					for(unsigned k = 0; k < ports.size(); k++)
						if (ports[k] && ports[k]->GetName() == name)
							found = true;
					if (!found)
						ports.push_back(new GOrgueMidiRtInPort(midi, GOrgueRtHelpers::GetMidiApiPrefix(apis[i]), name, apis[i]));
				}
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtMidi error: %s"), error.c_str());
			}
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}
