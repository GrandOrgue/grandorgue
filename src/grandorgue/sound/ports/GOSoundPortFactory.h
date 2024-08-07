/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPORTFACTORY_H
#define GOSOUNDPORTFACTORY_H

#include "config/GOPortFactory.h"
#include "sound/GOSoundDevInfo.h"

#include "GOSoundPort.h"

class GODeviceNamePattern;
class GOPortsConfig;
class GOSoundPort;

class GOSoundPortFactory : public GOPortFactory {
public:
  const std::vector<wxString> &GetPortNames() const;
  const std::vector<wxString> &GetPortApiNames(const wxString &portName) const;

  static std::vector<GOSoundDevInfo> getDeviceList(
    const GOPortsConfig &portsConfig);
  static GOSoundPort *create(
    const GOPortsConfig &portsConfig,
    GOSound *sound,
    GODeviceNamePattern &name);

  static GOSoundPortFactory &getInstance();
  static wxString getFullDeviceName(
    const wxString &portName,
    const wxString &apiName,
    const wxString &deviceName);
  static void terminate();
};

#endif /* GOSOUNDPORTFACTORY_H */
