/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDEVICECONFIG_H
#define GOMIDIDEVICECONFIG_H

#include <wx/string.h>

#include <vector>

#include "GODeviceNamePattern.h"

class GOMidiDeviceConfig : public GODeviceNamePattern {
private:
  void AssignMidiDeviceConfig(const GOMidiDeviceConfig &src);

public:
  typedef std::vector<GOMidiDeviceConfig *> RefVector;

  bool m_IsEnabled;

  // Midi-in only
  int m_ChannelShift = 0;
  GOMidiDeviceConfig *p_OutputDevice = NULL;

  GOMidiDeviceConfig(
    const wxString &logicalName,
    const wxString &regEx = wxEmptyString,
    const wxString &portName = wxEmptyString,
    const wxString &apiName = wxEmptyString,
    bool isEnabled = true,
    const wxString &physicalName = wxEmptyString);

  void Assign(const GOMidiDeviceConfig &src);

  GOMidiDeviceConfig(const GOMidiDeviceConfig &src) : GODeviceNamePattern(src) {
    AssignMidiDeviceConfig(src);
  }
};

#endif /* GOMIDIDEVICECONFIG_H */
