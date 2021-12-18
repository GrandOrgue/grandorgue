#ifndef GOMIDIRTPORTFACTORY_H
#define GOMIDIRTPORTFACTORY_H

#include <map>

#include "ptrvector.h"
#include "settings/GOPortsConfig.h"

#include "GOMidiPort.h"
#include "RtMidi.h"

class GOMidiRtPortFactory {
private:
  std::map<RtMidi::Api, RtMidiIn*> m_RtMidiIns;
  std::map<RtMidi::Api, RtMidiOut*> m_RtMidiOuts;
  
public:
  static const wxString PORT_NAME;

  GOMidiRtPortFactory();
  ~GOMidiRtPortFactory();
  
  void addMissingInDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiPort>& ports);
  void addMissingOutDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiPort>& ports);

  static wxString getApiName(RtMidi::Api api);
  static const std::vector<wxString> & getApis();
};

#endif /* GOMIDIRTPORTFACTORY_H */
