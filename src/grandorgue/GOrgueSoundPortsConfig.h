#ifndef GORGUESOUNDPORTSCONFIG_H
#define GORGUESOUNDPORTSCONFIG_H

#include <vector>
#include <wx/string.h>

class GOrgueSoundPortsConfig {
private:
  struct PortApiConfig {
    wxString m_PortName;
    wxString m_ApiName;
    bool m_IsEnabled;
    
    PortApiConfig(const wxString &portName, const wxString &apiName, bool isEnabled)
      : m_PortName(portName), m_ApiName(apiName), m_IsEnabled(isEnabled) {}
  };
  
  std::vector<PortApiConfig> m_PortApiConfigs;
  
  int Find(const wxString &portName, const wxString &apiName) const;
  
public:
  GOrgueSoundPortsConfig() {}
  GOrgueSoundPortsConfig(const GOrgueSoundPortsConfig &src): m_PortApiConfigs(src.m_PortApiConfigs) {}

  void Clear() { m_PortApiConfigs.clear(); }
  void AssignFrom(const GOrgueSoundPortsConfig &src) { m_PortApiConfigs = src.m_PortApiConfigs; }
  
  void SetConfigEnabled(const wxString &portName, const wxString &apiName, bool isEnabled);
  
  void SetConfigEnabled(const wxString &portName, bool isEnabled) 
  { SetConfigEnabled(portName, wxEmptyString, isEnabled); }
  
  // for setting
  bool IsConfigEnabled(const wxString &portName, const wxString &apiName) const;

  bool IsConfigEnabled(const wxString &portName) const
  { return IsConfigEnabled(portName, wxEmptyString); }
  
  // for using
  bool IsEnabled(const wxString &portName, const wxString &apiName) const
  {
    return IsConfigEnabled(portName, wxEmptyString)
      && IsConfigEnabled(portName, apiName);
  }
  
  bool IsEnabled(const wxString &portName) const 
  { return IsConfigEnabled(portName, wxEmptyString); }
};

#endif /* GORGUESOUNDPORTSCONFIG_H */

