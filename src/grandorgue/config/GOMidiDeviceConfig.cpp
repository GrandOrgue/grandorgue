/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDeviceConfig.h"

GOMidiDeviceConfig::GOMidiDeviceConfig(
  const wxString &logicalName,
  const wxString &regEx,
  const wxString &portName,
  const wxString &apiName,
  bool isEnabled,
  const wxString &physicalName)
  : GODeviceNamePattern(logicalName, regEx, portName, apiName, physicalName),
    m_IsEnabled(isEnabled) {}

void GOMidiDeviceConfig::AssignMidiDeviceConfig(const GOMidiDeviceConfig &src) {
  m_IsEnabled = src.m_IsEnabled;
  m_ChannelShift = src.m_ChannelShift;
  p_OutputDevice = NULL;
}

void GOMidiDeviceConfig::Assign(const GOMidiDeviceConfig &src) {
  AssignNamePattern(src);
  AssignMidiDeviceConfig(src);
}
