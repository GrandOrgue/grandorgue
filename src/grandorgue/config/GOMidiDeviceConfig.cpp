/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDeviceConfig.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

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

static const wxString WX_ENABLED = wxT("Enabled");
static const wxString WX_SHIFT = wxT("Shift");
static const wxString WX_OUTPUT_DEVICE = wxT("OutputDevice");

void GOMidiDeviceConfig::LoadDeviceConfig(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &prefix,
  const bool isInput) {
  LoadNamePattern(cfg, group, prefix, wxEmptyString);
  m_IsEnabled = cfg.ReadBoolean(CMBSetting, group, prefix + WX_ENABLED);
  if (isInput) {
    m_ChannelShift
      = cfg.ReadInteger(CMBSetting, group, prefix + WX_SHIFT, 0, 15);
    m_OutputDeviceName
      = cfg.ReadString(CMBSetting, group, prefix + WX_OUTPUT_DEVICE, false);
  } else {
    m_ChannelShift = 0;
    m_OutputDeviceName = wxEmptyString;
  }
}

void GOMidiDeviceConfig::SaveDeviceConfig(
  GOConfigWriter &cfg,
  const wxString &group,
  const wxString &prefix,
  const bool isInput) const {
  SaveNamePattern(cfg, group, prefix, wxEmptyString);
  cfg.WriteBoolean(group, prefix + WX_ENABLED, m_IsEnabled);
  if (isInput) {
    cfg.WriteInteger(group, prefix + WX_SHIFT, m_ChannelShift);
    if (p_OutputDevice)
      cfg.WriteString(
        group, prefix + WX_OUTPUT_DEVICE, p_OutputDevice->GetLogicalName());
  }
}
