/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPort.h"

#include "midi/GOMidi.h"
#include "midi/GOMidiMap.h"

static wxString GRANDORGUE = wxT("GrandOrgue");

const wxString GOMidiPort::GetClientName() { return GRANDORGUE; }

GOMidiPort::GOMidiPort(
  GOMidi *midi,
  const wxString &portName,
  const wxString &apiName,
  const wxString &deviceName,
  const wxString &fullName)
  : m_midi(midi),
    m_IsActive(false),
    m_PortName(portName),
    m_ApiName(apiName),
    m_DeviceName(deviceName),
    m_FullName(fullName) {}

bool GOMidiPort::IsToUse() const {
  return m_DeviceName.Find(GetClientName()) == wxNOT_FOUND;
}

bool GOMidiPort::IsEqualTo(
  const wxString &portName,
  const wxString &apiName,
  const wxString &deviceName) const {
  return m_PortName == portName && m_ApiName == apiName
    && m_DeviceName == deviceName;
}

bool GOMidiPort::Open(unsigned id) {
  m_ID = id;
  return true;
}
