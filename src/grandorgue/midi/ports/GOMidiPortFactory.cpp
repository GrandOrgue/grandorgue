/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiPortFactory.h"
#include "GOMidiRtPortFactory.h"

static bool hasPortsPopulated = false;
static std::vector<wxString> portNames;

const std::vector<wxString>& GOMidiPortFactory::GetPortNames() const
{
  if (! hasPortsPopulated)
  {
    portNames.push_back(GOMidiRtPortFactory::PORT_NAME);
    hasPortsPopulated = true;
  }
  return portNames;
}

const std::vector<wxString>& GOMidiPortFactory::GetPortApiNames(const wxString & portName)
const
{
  if (portName == GOMidiRtPortFactory::PORT_NAME)
    return GOMidiRtPortFactory::getApis();
  else // old-style name
    return c_NoApis;
}

static GOMidiPortFactory instance;

GOMidiPortFactory& GOMidiPortFactory::getInstance() { return instance; }

static GOMidiRtPortFactory* p_rt_factory = NULL;

GOMidiRtPortFactory* get_rt()
{
  if (! p_rt_factory)
    p_rt_factory = new GOMidiRtPortFactory();
  return p_rt_factory;
}

void GOMidiPortFactory::addMissingInDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiInPort>& ports)
{
  if (portsConfig.IsEnabled(GOMidiRtPortFactory::PORT_NAME))
    get_rt()->addMissingInDevices(midi, portsConfig, ports);
}

void GOMidiPortFactory::addMissingOutDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiOutPort>& ports)
{
  if (portsConfig.IsEnabled(GOMidiRtPortFactory::PORT_NAME))
    get_rt() -> addMissingOutDevices(midi, portsConfig, ports);
}

void GOMidiPortFactory::terminate()
{
  if (p_rt_factory)
  {
    delete p_rt_factory;
    p_rt_factory = NULL;
  }
}
