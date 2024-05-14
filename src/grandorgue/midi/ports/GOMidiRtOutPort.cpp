/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiRtOutPort.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOMidiRtPortFactory.h"

GOMidiRtOutPort::GOMidiRtOutPort(
  GOMidi *midi,
  RtMidi::Api api,
  const wxString &deviceName,
  const wxString &fullName)
  : GOMidiOutPort(
    midi,
    GOMidiRtPortFactory::PORT_NAME,
    GOMidiRtPortFactory::getApiName(api),
    deviceName,
    fullName),
    m_api(api),
    m_port(NULL) {}

GOMidiRtOutPort::~GOMidiRtOutPort() { Close(true); }

const wxString GOMidiRtOutPort::GetDefaultLogicalName() const {
  return GOMidiRtPortFactory::getInstance()->GetDefaultLogicalName(
    m_api, GetDeviceName(), GetName());
}

const wxString GOMidiRtOutPort::GetDefaultRegEx() const {
  return GOMidiRtPortFactory::getInstance()->GetDefaultRegEx(
    m_api, GetDeviceName(), GetName());
}

bool GOMidiRtOutPort::Open(unsigned id) {
  Close(false);
  if (!m_port)
    try {
      m_port = new RtMidiOut(m_api, (const char *)GetClientName().fn_str());
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
  return GOMidiOutPort::Open(id);
}

void GOMidiRtOutPort::Close(bool isToFreePort) {
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
    RtMidiOut *const port = m_port;

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

void GOMidiRtOutPort::SendData(std::vector<unsigned char> &msg) {
  if (m_port)
    try {
      m_port->sendMessage(&msg);
    } catch (RtMidiError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}
