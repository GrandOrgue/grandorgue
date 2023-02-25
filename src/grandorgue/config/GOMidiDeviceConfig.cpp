/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDeviceConfig.h"

GOMidiDeviceConfig::GOMidiDeviceConfig(
  const wxString &logicalName,
  const wxString &regEx,
  const wxString portName,
  const wxString apiName,
  bool isEnabled,
  const wxString &physicalName)
  : m_LogicalName(logicalName),
    m_PortName(portName),
    m_ApiName(apiName),
    m_IsEnabled(isEnabled),
    m_PhysicalName(physicalName) {
  SetRegEx(regEx);
}

void GOMidiDeviceConfig::SetRegEx(const wxString &regEx) {
  if (regEx != m_RegEx) {
    if (p_CompiledRegEx)
      delete p_CompiledRegEx;
    m_RegEx = regEx;
    p_CompiledRegEx = regEx.IsEmpty() ? NULL : new wxRegEx(regEx);
  }
}

void GOMidiDeviceConfig::Assign(const GOMidiDeviceConfig &src) {
  m_LogicalName = src.m_LogicalName;
  m_PortName = src.m_PortName;
  m_ApiName = src.m_ApiName;
  SetRegEx(src.m_RegEx);
  m_IsEnabled = src.m_IsEnabled;
  m_ChannelShift = src.m_ChannelShift;
  m_PhysicalName = src.m_PhysicalName;
  p_OutputDevice = NULL;
}

bool GOMidiDeviceConfig::DoesMatch(const wxString &physicalName) {
  return (p_CompiledRegEx && p_CompiledRegEx->IsValid()
          && p_CompiledRegEx->Matches(physicalName))
    || m_LogicalName == physicalName;
}
