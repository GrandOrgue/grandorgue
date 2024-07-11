/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODeviceNamePattern.h"

#include <wx/regex.h>

GODeviceNamePattern::GODeviceNamePattern(
  const wxString &logicalName,
  const wxString &regEx,
  const wxString &portName,
  const wxString &apiName,
  const wxString &physicalName)
  : m_LogicalName(logicalName),
    m_PortName(portName),
    m_ApiName(apiName),
    m_PhysicalName(physicalName) {
  SetRegEx(regEx);
}

GODeviceNamePattern::~GODeviceNamePattern() {
  if (m_CompiledRegEx)
    delete m_CompiledRegEx;
}

void GODeviceNamePattern::AssignNamePattern(const GODeviceNamePattern &src) {
  m_LogicalName = src.m_LogicalName;
  m_PortName = src.m_PortName;
  m_ApiName = src.m_ApiName;
  SetRegEx(src.m_RegEx);
  m_PhysicalName = src.m_PhysicalName;
}

void GODeviceNamePattern::SetRegEx(const wxString &regEx) {
  if (regEx != m_RegEx) {
    if (m_CompiledRegEx)
      delete m_CompiledRegEx;
    m_RegEx = regEx;
    m_CompiledRegEx = regEx.IsEmpty() ? nullptr : new wxRegEx(regEx);
  }
}

bool GODeviceNamePattern::DoesMatch(const wxString &physicalName) {
  return (m_CompiledRegEx && m_CompiledRegEx->IsValid()
          && m_CompiledRegEx->Matches(physicalName))
    || m_LogicalName == physicalName;
}
