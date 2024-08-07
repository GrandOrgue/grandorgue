/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundPortFactory.h"

#include "GOSoundJackPort.h"
#include "GOSoundPortaudioPort.h"
#include "GOSoundRtPort.h"
#include "config/GODeviceNamePattern.h"

static bool hasPortsPopulated = false;
static std::vector<wxString> portNames;

const std::vector<wxString> &GOSoundPortFactory::GetPortNames() const {
  if (!hasPortsPopulated) {
    portNames.push_back(GOSoundPortaudioPort::PORT_NAME);
    portNames.push_back(GOSoundRtPort::PORT_NAME);
#if defined(GO_USE_JACK)
    portNames.push_back(GOSoundJackPort::PORT_NAME);
#endif
    hasPortsPopulated = true;
  }
  return portNames;
}

const std::vector<wxString> &GOSoundPortFactory::GetPortApiNames(
  const wxString &portName) const {
  if (portName == GOSoundPortaudioPort::PORT_NAME)
    return GOSoundPortaudioPort::getApis();
  else if (portName == GOSoundRtPort::PORT_NAME)
    return GOSoundRtPort::getApis();
  else if (portName == GOSoundJackPort::PORT_NAME)
    return GOSoundJackPort::getApis();
  else // old-style name
    return c_NoApis;
}

enum { SUBSYS_PA_BIT = 1, SUBSYS_RT_BIT = 2, SUBSYS_JACK_BIT = 4 };

GOSoundPort *GOSoundPortFactory::create(
  const GOPortsConfig &portsConfig,
  GOSound *sound,
  GODeviceNamePattern &pattern) {
  GOSoundPort *port = NULL;
  const wxString &patternPort = pattern.GetPortName();
  wxString portName = !patternPort.IsEmpty()
    ? patternPort
    : NameParser(pattern.GetLogicalName()).nextComp();
  unsigned short portMask; // possible ports matching with the name

  if (
    portName == GOSoundPortaudioPort::PORT_NAME
    || portName == GOSoundPortaudioPort::PORT_NAME_OLD)
    portMask = SUBSYS_PA_BIT;
  else if (
    portName == GOSoundRtPort::PORT_NAME
    || portName == GOSoundRtPort::PORT_NAME_OLD)
    portMask = SUBSYS_RT_BIT;
  else if (portName == GOSoundJackPort::PORT_NAME)
    portMask = SUBSYS_JACK_BIT;
  else // old-style name
    portMask = SUBSYS_PA_BIT | SUBSYS_RT_BIT | SUBSYS_JACK_BIT;

  if (
    port == NULL && (portMask & SUBSYS_PA_BIT)
    && portsConfig.IsEnabled(GOSoundPortaudioPort::PORT_NAME))
    port = GOSoundPortaudioPort::create(portsConfig, sound, pattern);
  if (
    port == NULL && (portMask & SUBSYS_RT_BIT)
    && portsConfig.IsEnabled(GOSoundRtPort::PORT_NAME))
    port = GOSoundRtPort::create(portsConfig, sound, pattern);
  if (
    port == NULL && (portMask & SUBSYS_JACK_BIT)
    && portsConfig.IsEnabled(GOSoundJackPort::PORT_NAME))
    port = GOSoundJackPort::create(portsConfig, sound, pattern);
  return port;
}

std::vector<GOSoundDevInfo> GOSoundPortFactory::getDeviceList(
  const GOPortsConfig &portsConfig) {
  std::vector<GOSoundDevInfo> result;

  if (portsConfig.IsEnabled(GOSoundPortaudioPort::PORT_NAME))
    GOSoundPortaudioPort::addDevices(portsConfig, result);
  if (portsConfig.IsEnabled(GOSoundRtPort::PORT_NAME))
    GOSoundRtPort::addDevices(portsConfig, result);
  if (portsConfig.IsEnabled(GOSoundJackPort::PORT_NAME))
    GOSoundJackPort::addDevices(portsConfig, result);
  return result;
}

static GOSoundPortFactory instance;

GOSoundPortFactory &GOSoundPortFactory::getInstance() { return instance; }

wxString GOSoundPortFactory::getFullDeviceName(
  const wxString &portName,
  const wxString &apiName,
  const wxString &deviceName) {
  return instance.ComposeDeviceName(portName, apiName, deviceName);
}

void GOSoundPortFactory::terminate() { GOSoundPortaudioPort::terminate(); }
