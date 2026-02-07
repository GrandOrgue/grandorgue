/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPORTFACTORY_H
#define GOMIDIPORTFACTORY_H

#include <ptrvector.h>

#include "GOMidiPort.h"
#include "config/GOPortFactory.h"
#include "config/GOPortsConfig.h"

class GOMidiSystem;

class GOMidiPortFactory : public GOPortFactory {
public:
  virtual const std::vector<wxString> &GetPortNames() const;
  virtual const std::vector<wxString> &GetPortApiNames(
    const wxString &portName) const;

  static GOMidiPortFactory &getInstance();
  static void addMissingInDevices(
    GOMidiSystem *midi,
    const GOPortsConfig &portsConfig,
    ptr_vector<GOMidiPort> &ports);
  static void addMissingOutDevices(
    GOMidiSystem *midi,
    const GOPortsConfig &portsConfig,
    ptr_vector<GOMidiPort> &ports);
  static void terminate();
};

#endif /* GOMIDIPORTFACTORY_H */
