/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPORTSCONFIG_H
#define GOPORTSCONFIG_H

#include <wx/string.h>

#include <vector>

class GOPortsConfig {
private:
  struct PortApiConfig {
    wxString m_PortName;
    wxString m_ApiName;
    bool m_IsEnabled;

    PortApiConfig(
      const wxString &portName, const wxString &apiName, bool isEnabled)
      : m_PortName(portName), m_ApiName(apiName), m_IsEnabled(isEnabled) {}

    bool operator==(const PortApiConfig &other) const {
      return m_PortName == other.m_PortName && m_ApiName == other.m_ApiName
        && m_IsEnabled == other.m_IsEnabled;
    }

    bool operator!=(const PortApiConfig &other) const {
      return m_PortName != other.m_PortName || m_ApiName != other.m_ApiName
        || m_IsEnabled != other.m_IsEnabled;
    }
  };

  std::vector<PortApiConfig> m_PortApiConfigs;

  int Find(const wxString &portName, const wxString &apiName) const;

public:
  GOPortsConfig() {}
  GOPortsConfig(const GOPortsConfig &src)
    : m_PortApiConfigs(src.m_PortApiConfigs) {}

  bool operator==(const GOPortsConfig &other) const {
    return m_PortApiConfigs == other.m_PortApiConfigs;
  }
  bool operator!=(const GOPortsConfig &other) const {
    return m_PortApiConfigs != other.m_PortApiConfigs;
  }

  void Clear() { m_PortApiConfigs.clear(); }
  void AssignFrom(const GOPortsConfig &src) {
    m_PortApiConfigs = src.m_PortApiConfigs;
  }

  void SetConfigEnabled(
    const wxString &portName, const wxString &apiName, bool isEnabled);

  void SetConfigEnabled(const wxString &portName, bool isEnabled) {
    SetConfigEnabled(portName, wxEmptyString, isEnabled);
  }

  // for setting
  bool IsConfigEnabled(const wxString &portName, const wxString &apiName) const;

  bool IsConfigEnabled(const wxString &portName) const {
    return IsConfigEnabled(portName, wxEmptyString);
  }

  // for using
  bool IsEnabled(const wxString &portName, const wxString &apiName) const {
    return IsConfigEnabled(portName, wxEmptyString)
      && IsConfigEnabled(portName, apiName);
  }

  bool IsEnabled(const wxString &portName) const {
    return IsConfigEnabled(portName, wxEmptyString);
  }
};

#endif /* GOSOUNDPORTSCONFIG_H */
