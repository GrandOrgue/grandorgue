#ifndef GORGUEMIDIRTFACTORY_H
#define GORGUEMIDIRTFACTORY_H

#include <map>

#include "GOrgueMidiInPort.h"
#include "GOrgueMidiOutPort.h"
#include "RtMidi.h"

class GOrgueMidiRtFactory {
private:
  std::map<RtMidi::Api, RtMidiIn*> m_RtMidiIns;
  std::map<RtMidi::Api, RtMidiOut*> m_RtMidiOuts;
  
public:
  GOrgueMidiRtFactory();
  ~GOrgueMidiRtFactory();
  
  void addMissingInDevices(GOrgueMidi* midi, ptr_vector<GOrgueMidiInPort>& ports);
  void addMissingOutDevices(GOrgueMidi* midi, ptr_vector<GOrgueMidiOutPort>& ports);
};

#endif /* GORGUEMIDIRTFACTORY_H */

