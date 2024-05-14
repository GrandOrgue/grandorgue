/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDEVICECONFIG_H
#define GOMIDIDEVICECONFIG_H

#include <wx/regex.h>
#include <wx/string.h>

#include <vector>

class GOMidiDeviceConfig {
private:
  wxRegEx *p_CompiledRegEx = NULL;

public:
  typedef std::vector<GOMidiDeviceConfig *> RefVector;

  wxString m_LogicalName;
  wxString m_RegEx;
  wxString m_PortName;
  wxString m_ApiName;
  bool m_IsEnabled;

  // Midi-in only
  int m_ChannelShift = 0;
  GOMidiDeviceConfig *p_OutputDevice = NULL;

  // matchingResult. Not saved, only in memory
  wxString m_PhysicalName;

  GOMidiDeviceConfig(
    const wxString &logicalName,
    const wxString &regEx = wxEmptyString,
    const wxString portName = wxEmptyString,
    const wxString apiName = wxEmptyString,
    bool isEnabled = true,
    const wxString &physicalName = wxEmptyString);

  void Assign(const GOMidiDeviceConfig &src);

  GOMidiDeviceConfig(const GOMidiDeviceConfig &src) { Assign(src); }

  void SetRegEx(const wxString &regEx);

  bool DoesMatch(const wxString &physicalName);
};

#endif /* GOMIDIDEVICECONFIG_H */
