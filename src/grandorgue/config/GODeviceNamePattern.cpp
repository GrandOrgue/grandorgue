/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODeviceNamePattern.h"

#include <wx/regex.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

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

static const wxString WX_REGEX = wxT("RegEx");
static const wxString WX_PORT_NAME = wxT("PortName");
static const wxString WX_API_NAME = wxT("ApiName");

void GODeviceNamePattern::LoadNamePattern(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &prefix,
  const wxString &nameKey) {
  m_LogicalName = cfg.ReadString(CMBSetting, group, prefix + nameKey),
  SetRegEx(cfg.ReadString(CMBSetting, group, prefix + WX_REGEX, false));
  m_PortName = cfg.ReadString(CMBSetting, group, prefix + WX_PORT_NAME, false);
  m_ApiName = cfg.ReadString(CMBSetting, group, prefix + WX_API_NAME, false);
}

void GODeviceNamePattern::SaveNamePattern(
  GOConfigWriter &cfg,
  const wxString &group,
  const wxString &prefix,
  const wxString &nameKey) const {
  cfg.WriteString(group, prefix + nameKey, m_LogicalName);
  cfg.WriteString(group, prefix + WX_REGEX, m_RegEx);
  cfg.WriteString(group, prefix + WX_PORT_NAME, m_PortName);
  cfg.WriteString(group, prefix + WX_API_NAME, m_ApiName);
}

bool GODeviceNamePattern::DoesMatch(const wxString &physicalName) const {
  return (m_CompiledRegEx && m_CompiledRegEx->IsValid()
          && m_CompiledRegEx->Matches(physicalName))
    || m_LogicalName == physicalName;
}
