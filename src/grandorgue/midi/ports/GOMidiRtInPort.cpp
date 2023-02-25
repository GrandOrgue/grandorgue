/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiRtInPort.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOMidiRtPortFactory.h"

GOMidiRtInPort::GOMidiRtInPort(
  GOMidi *midi,
  RtMidi::Api api,
  const wxString &deviceName,
  const wxString &fullName)
  : GOMidiInPort(
    midi,
    GOMidiRtPortFactory::PORT_NAME,
    GOMidiRtPortFactory::getApiName(api),
    deviceName,
    fullName),
    m_api(api),
    m_port(NULL) {}

GOMidiRtInPort::~GOMidiRtInPort() { Close(true); }

const wxString GOMidiRtInPort::GetDefaultLogicalName() const {
  return GOMidiRtPortFactory::getInstance()->GetDefaultLogicalName(
    m_api, GetDeviceName(), GetName());
}

const wxString GOMidiRtInPort::GetDefaultRegEx() const {
  return GOMidiRtPortFactory::getInstance()->GetDefaultRegEx(
    m_api, GetDeviceName(), GetName());
}

bool GOMidiRtInPort::Open(unsigned id, int channel_shift) {
  Close(false);
  if (!m_port)
    try {
      m_port = new RtMidiIn(m_api, (const char *)GetClientName().fn_str());
      m_port->ignoreTypes(false);
      m_port->setCallback(&MIDICallback, this);
    } catch (RtMidiError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
  if (!m_port)
    return false;
  try {
    for (unsigned i = 0; i < m_port->getPortCount(); i++) {
      if (m_DeviceName == wxString::FromAscii(m_port->getPortName(i).c_str())) {
        m_port->openPort(i, (const char *)GetMyNativePortName().fn_str());
        m_IsActive = true;
        break;
      }
    }
  } catch (RtMidiError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtMidi error: %s"), error.c_str());
  }
  return GOMidiInPort::Open(id, channel_shift);
}

void GOMidiRtInPort::Close(bool isToFreePort) {
  if (m_IsActive) {
    if (m_port)
      try {
        m_port->closePort();
      } catch (RtMidiError &e) {
        wxString error = wxString::FromAscii(e.getMessage().c_str());
        wxLogError(_("RtMidi error: %s"), error.c_str());
      }
    m_IsActive = false;
  }
  if (isToFreePort) {
    RtMidiIn *const port = m_port;

    if (port) {
      m_port = NULL;
      try {
        delete port;
      } catch (RtMidiError &e) {
        wxString error = wxString::FromAscii(e.getMessage().c_str());
        wxLogError(_("RtMidi error: %s"), error.c_str());
      }
    }
  }
}

void GOMidiRtInPort::MIDICallback(
  double timeStamp, std::vector<unsigned char> *msg, void *userData) {
  GOMidiRtInPort *port = (GOMidiRtInPort *)userData;

  if (port->m_IsActive && port->m_port)
    port->Receive(*msg);
}
