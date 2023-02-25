/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPortsConfig.h"

int GOPortsConfig::Find(
  const wxString &portName, const wxString &apiName) const {
  int res = -1;

  for (size_t i = 0, l = m_PortApiConfigs.size(); i < l; i++) {
    const PortApiConfig &c(m_PortApiConfigs[i]);

    if (c.m_PortName == portName && c.m_ApiName == apiName) {
      res = i;
      break;
    }
  }
  return res;
}

void GOPortsConfig::SetConfigEnabled(
  const wxString &portName, const wxString &apiName, bool isEnabled) {
  const PortApiConfig c(portName, apiName, isEnabled);
  const int i = Find(portName, apiName);

  if (i >= 0)
    m_PortApiConfigs[i] = c;
  else
    m_PortApiConfigs.push_back(c);
}

bool GOPortsConfig::IsConfigEnabled(
  const wxString &portName, const wxString &apiName) const {
  const int i = Find(portName, apiName);

  return (i < 0) || m_PortApiConfigs[i].m_IsEnabled;
}
