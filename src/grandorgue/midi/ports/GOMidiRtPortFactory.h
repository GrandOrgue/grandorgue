#ifndef GOMIDIRTPORTFACTORY_H
#define GOMIDIRTPORTFACTORY_H

#include <map>

#include "GOMidiInPort.h"
#include "GOMidiOutPort.h"
#include "RtMidi.h"

class GOMidiRtPortFactory {
private:
  std::map<RtMidi::Api, RtMidiIn*> m_RtMidiIns;
  std::map<RtMidi::Api, RtMidiOut*> m_RtMidiOuts;
  
public:
  static const wxString PORT_NAME;

  GOMidiRtPortFactory();
  ~GOMidiRtPortFactory();
  
  void addMissingInDevices(GOMidi* midi, ptr_vector<GOMidiInPort>& ports);
  void addMissingOutDevices(GOMidi* midi, ptr_vector<GOMidiOutPort>& ports);

  static const std::vector<wxString> & getApis();
};

#endif /* GOMIDIRTPORTFACTORY_H */
