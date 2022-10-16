/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRTPORT_H
#define GOSOUNDRTPORT_H

#include "GOSoundPort.h"
#include "RtAudio.h"
#include "sound/GOSoundDevInfo.h"

class GOSoundRtPort : public GOSoundPort {
private:
  RtAudio *m_rtApi;
  unsigned m_nBuffers;

  static int Callback(
    void *outputBuffer,
    void *inputBuffer,
    unsigned int nFrames,
    double streamTime,
    RtAudioStreamStatus status,
    void *userData);

  static wxString getName(RtAudio *rtApi, unsigned index);

public:
  static const wxString PORT_NAME;
  static const wxString PORT_NAME_OLD;

  // rtApi to be deleted in the destructor
  GOSoundRtPort(GOSound *sound, RtAudio *rtApi, wxString name);
  ~GOSoundRtPort();

  void Open();
  void StartStream();
  void Close();

  static const std::vector<wxString> &getApis();
  static GOSoundPort *create(
    const GOPortsConfig &portsConfig, GOSound *sound, wxString name);
  static void addDevices(
    const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &list);
};

#endif
