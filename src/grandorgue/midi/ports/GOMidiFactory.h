/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIFACTORY_H
#define GOMIDIFACTORY_H

#include <ptrvector.h>

#include "GOMidiInPort.h"
#include "GOMidiOutPort.h"
#include "GOMidiRtFactory.h"

class GOMidi;

class GOMidiFactory {
private:
  GOMidiRtFactory m_RtFactory;

public:
  void addMissingInDevices(GOMidi* midi, ptr_vector<GOMidiInPort>& ports);
  void addMissingOutDevices(GOMidi* midi, ptr_vector<GOMidiOutPort>& ports);
};

#endif /* GOMIDIFACTORY_H */

