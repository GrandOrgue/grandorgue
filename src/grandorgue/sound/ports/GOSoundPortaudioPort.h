/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPORTAUDIOPORT_H
#define GOSOUNDPORTAUDIOPORT_H

#include "GOSoundPort.h"
#include "GOSoundPortFactory.h"
#include "portaudio.h"

class GOSoundPortaudioPort : public GOSoundPort {
private:
  PaStream *m_stream;

  static int Callback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData);

  static wxString getName(unsigned index);
  static wxString getLastError(PaError error);

public:
  static const wxString PORT_NAME;
  static const wxString PORT_NAME_OLD;

  GOSoundPortaudioPort(GOSound *sound, wxString name);
  virtual ~GOSoundPortaudioPort();

  void Open();
  void StartStream();
  void Close();

  static const std::vector<wxString> &getApis() {
    return GOSoundPortFactory::c_NoApis;
  }
  static GOSoundPort *create(
    const GOPortsConfig &portsConfig, GOSound *sound, wxString name);
  static void addDevices(
    const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &list);
  static void terminate();
};

#endif
