/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDPORTFACTORY_H
#define GOSOUNDPORTFACTORY_H

#include "settings/GOPortFactory.h"
#include "settings/GOPortsConfig.h"
#include "sound/GOSoundDevInfo.h"
#include "GOSoundPort.h"

class GOSoundPortFactory: public GOPortFactory
{
public:
  const std::vector<wxString>& GetPortNames() const;
  const std::vector<wxString>& GetPortApiNames(const wxString & portName) const;

  static std::vector<GOSoundDevInfo> getDeviceList(const GOPortsConfig &portsConfig);
  static GOSoundPort* create(const GOPortsConfig &portsConfig, GOSound* sound, wxString name);

  static GOSoundPortFactory& getInstance();
  static void terminate();
};

#endif /* GOSOUNDPORTFACTORY_H */

