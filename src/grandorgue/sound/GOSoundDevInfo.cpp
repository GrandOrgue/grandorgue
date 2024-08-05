/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundDevInfo.h"

#include "ports/GOSoundPortFactory.h"

GOSoundDevInfo::GOSoundDevInfo(
  const wxString &portName,
  const wxString &apiName,
  const wxString &name,
  const wxString &fullName,
  unsigned channels,
  bool isDefault,
  void fillLogicalNameAndRegEx(GOSoundDevInfo &info))
  : m_PortName(portName),
    m_ApiName(apiName),
    m_name(name),
    m_FullName(fullName),
    m_DefaultLogicalName(fullName),
    m_DefaultNameRegex(),
    m_channels(channels),
    m_IsDefault(isDefault) {
  if (fillLogicalNameAndRegEx)
    fillLogicalNameAndRegEx(*this);
}

GOSoundDevInfo::GOSoundDevInfo(
  const wxString &portName,
  const wxString &apiName,
  const wxString &name,
  unsigned channels,
  bool isDefault,
  void fillLogicalNameAndRegEx(GOSoundDevInfo &info))
  : GOSoundDevInfo(
    portName,
    apiName,
    name,
    GOSoundPortFactory::getFullDeviceName(portName, apiName, name),
    channels,
    isDefault,
    fillLogicalNameAndRegEx) {}

/* Don't use the short constructor here because it calls to
 * GOSoundPortFactory::getFullDeviceName that uses static variables from other
 * units. The order of initializing static variables in different units is not
 * defined
 */
const GOSoundDevInfo GOSoundDevInfo::INVALID_DEVICE_INFO(
  wxEmptyString,
  wxEmptyString,
  wxEmptyString,
  wxEmptyString,
  0,
  false,
  nullptr);
