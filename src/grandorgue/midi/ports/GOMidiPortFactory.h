/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIPORTFACTORY_H
#define GOMIDIPORTFACTORY_H

#include <ptrvector.h>

#include "settings/GOPortFactory.h"
#include "settings/GOPortsConfig.h"

#include "GOMidiInPort.h"
#include "GOMidiOutPort.h"

class GOMidi;

class GOMidiPortFactory: public GOPortFactory
{
public:
  virtual const std::vector<wxString>& GetPortNames() const;
  virtual const std::vector<wxString>& GetPortApiNames(const wxString & portName) const;

  static GOMidiPortFactory& getInstance();
  static void addMissingInDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiInPort>& ports);
  static void addMissingOutDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiOutPort>& ports);
  static void terminate();
};

#endif /* GOMIDIPORTFACTORY_H */

