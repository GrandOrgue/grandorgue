/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiRtInPort.h"
#include <wx/intl.h>
#include <wx/log.h>

GOrgueMidiRtInPort::GOrgueMidiRtInPort(GOrgueMidi* midi, wxString prefix, wxString name, RtMidi::Api api) :
  GOrgueMidiInPort(midi, prefix, name),
  m_api(api),
  m_port(NULL)
{
}

GOrgueMidiRtInPort::~GOrgueMidiRtInPort()
{
  Close(true);
}

bool GOrgueMidiRtInPort::Open(int channel_shift)
{
  Close(false);
  if (!m_port)
    try
    {
      m_port = new RtMidiIn(m_api, (const char*)GetClientName().fn_str());
      m_port->ignoreTypes(false);
      m_port->setCallback(&MIDICallback, this);
    }
    catch (RtMidiError &e)
    {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
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
  catch (RtMidiError &e)
  {
	  wxString error = wxString::FromAscii(e.getMessage().c_str());
	  wxLogError(_("RtMidi error: %s"), error.c_str());
  }
  return GOrgueMidiInPort::Open(channel_shift);
}

void GOrgueMidiRtInPort::Close(bool isToFreePort)
{
  if (m_IsActive)
  {
    if (m_port)
      try
      {
	m_port->closePort();
      }
      catch (RtMidiError &e)
      {
	wxString error = wxString::FromAscii(e.getMessage().c_str());
	wxLogError(_("RtMidi error: %s"), error.c_str());
      }
    m_IsActive = false;
  }
  if (isToFreePort)
  {
    RtMidiIn* const port = m_port;

    if (port)
    {
      m_port = NULL;
      try
      {
	delete port;
      }
      catch (RtMidiError &e)
      {
	wxString error = wxString::FromAscii(e.getMessage().c_str());
	wxLogError(_("RtMidi error: %s"), error.c_str());
      }
    }
  }
}

void GOrgueMidiRtInPort::MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData)
{
  GOrgueMidiRtInPort* port = (GOrgueMidiRtInPort*)userData;

  if (port->m_IsActive && port->m_port)
    port->Receive(*msg);
}
