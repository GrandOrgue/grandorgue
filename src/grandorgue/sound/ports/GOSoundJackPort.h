/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDJACKPORT_H
#define GOSOUNDJACKPORT_H

#if defined(GO_USE_JACK)
#if defined(_WIN32) && !defined(WIN32)
// workaround of the bug in the old jack that was fixed in
// https://github.com/jackaudio/jack2/commit/49f06118a105f480634d9728ffdd4bfe2310860a
// otherwise it couldnt compiled with mingw
#define WIN32 1
#endif
#include <jack/jack.h>
#endif

#include "GOSoundPort.h"
#include "GOSoundPortFactory.h"

class GOSoundJackPort : public GOSoundPort {
public:
  static const wxString PORT_NAME;

  GOSoundJackPort(GOSound *sound, wxString name);
  ~GOSoundJackPort();

#if defined(GO_USE_JACK)
private:
  jack_client_t *m_JackClient = NULL;
  jack_port_t **m_JackOutputPorts = NULL;
  float *m_GoBuffer = NULL;
  bool m_IsOpen = false;
  bool m_IsStarted = false;

  static void JackLatencyCallback(
    jack_latency_callback_mode_t mode, void *data);
  static int JackProcessCallback(jack_nframes_t nFrames, void *data);
  static void JackShutdownCallback(void *data);

  static wxString getName();

public:
  void Open();
  void StartStream();
#endif /* GO_USE_JACK */

public:
  void Close();

  static const std::vector<wxString> &getApis() {
    return GOSoundPortFactory::c_NoApis;
  }
  static GOSoundPort *create(
    const GOPortsConfig &portsConfig, GOSound *sound, wxString name);
  static void addDevices(
    const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &list);
};

#endif /* GOSOUNDJACKPORT_H */
