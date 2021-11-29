#ifndef GOMIDIRTFACTORY_H
#define GOMIDIRTFACTORY_H

#include <map>

#include "GOMidiInPort.h"
#include "GOMidiOutPort.h"
#include "RtMidi.h"

class GOMidiRtFactory {
private:
  std::map<RtMidi::Api, RtMidiIn*> m_RtMidiIns;
  std::map<RtMidi::Api, RtMidiOut*> m_RtMidiOuts;
  
public:
  GOMidiRtFactory();
  ~GOMidiRtFactory();
  
  void addMissingInDevices(GOMidi* midi, ptr_vector<GOMidiInPort>& ports);
  void addMissingOutDevices(GOMidi* midi, ptr_vector<GOMidiOutPort>& ports);
};

#endif /* GOMIDIRTFACTORY_H */
