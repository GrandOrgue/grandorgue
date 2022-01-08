#ifndef GOMIDIRTPORTFACTORY_H
#define GOMIDIRTPORTFACTORY_H

#include <wx/regex.h>

#include <map>

#include "GOMidiPort.h"
#include "RtMidi.h"
#include "config/GOPortsConfig.h"
#include "ptrvector.h"

class GOMidiRtPortFactory {
private:
  std::map<RtMidi::Api, RtMidiIn *> m_RtMidiIns;
  std::map<RtMidi::Api, RtMidiOut *> m_RtMidiOuts;

  // patterns for parsing device names of different apis,
  wxRegEx m_AlsaDevnamePattern;
  wxRegEx m_JackDevnamePattern;
  wxRegEx m_WinMmDevnamePattern;

public:
  static const wxString PORT_NAME;

  GOMidiRtPortFactory();
  ~GOMidiRtPortFactory();

  static GOMidiRtPortFactory *getInstance();
  static void terminateInstance();

  void addMissingInDevices(
    GOMidi *midi,
    const GOPortsConfig &portsConfig,
    ptr_vector<GOMidiPort> &ports);
  void addMissingOutDevices(
    GOMidi *midi,
    const GOPortsConfig &portsConfig,
    ptr_vector<GOMidiPort> &ports);

  static wxString getApiName(RtMidi::Api api);
  static const std::vector<wxString> &getApis();

  wxString GetDefaultLogicalName(
    RtMidi::Api api, const wxString &deviceName, const wxString &fullName);
  wxString GetDefaultRegEx(
    RtMidi::Api api, const wxString &deviceName, const wxString &fullName);
};

#endif /* GOMIDIRTPORTFACTORY_H */
