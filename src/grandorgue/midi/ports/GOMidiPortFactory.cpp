/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPortFactory.h"

#include "GOMidiRtPortFactory.h"

static bool hasPortsPopulated = false;
static std::vector<wxString> portNames;

const std::vector<wxString> &GOMidiPortFactory::GetPortNames() const {
  if (!hasPortsPopulated) {
    portNames.push_back(GOMidiRtPortFactory::PORT_NAME);
    hasPortsPopulated = true;
  }
  return portNames;
}

const std::vector<wxString> &GOMidiPortFactory::GetPortApiNames(
  const wxString &portName) const {
  if (portName == GOMidiRtPortFactory::PORT_NAME)
    return GOMidiRtPortFactory::getApis();
  else // old-style name
    return c_NoApis;
}

static GOMidiPortFactory instance;

GOMidiPortFactory &GOMidiPortFactory::getInstance() { return instance; }

void GOMidiPortFactory::addMissingInDevices(
  GOMidi *midi,
  const GOPortsConfig &portsConfig,
  ptr_vector<GOMidiPort> &ports) {
  if (portsConfig.IsEnabled(GOMidiRtPortFactory::PORT_NAME))
    GOMidiRtPortFactory::getInstance()->addMissingInDevices(
      midi, portsConfig, ports);
}

void GOMidiPortFactory::addMissingOutDevices(
  GOMidi *midi,
  const GOPortsConfig &portsConfig,
  ptr_vector<GOMidiPort> &ports) {
  if (portsConfig.IsEnabled(GOMidiRtPortFactory::PORT_NAME))
    GOMidiRtPortFactory::getInstance()->addMissingOutDevices(
      midi, portsConfig, ports);
}

void GOMidiPortFactory::terminate() {
  GOMidiRtPortFactory::terminateInstance();
}
