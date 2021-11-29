/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiPortFactory.h"

void GOMidiFactory::addMissingInDevices(GOMidi* midi, ptr_vector<GOMidiInPort>& ports)
{
  m_RtFactory.addMissingInDevices(midi, ports);
}

void GOMidiFactory::addMissingOutDevices(GOMidi* midi, ptr_vector<GOMidiOutPort>& ports)
{
  m_RtFactory.addMissingOutDevices(midi, ports);
}