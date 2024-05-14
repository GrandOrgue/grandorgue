/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPORT_H
#define GOSOUNDPORT_H

#include <wx/string.h>

#include <vector>

#include "config/GOPortsConfig.h"

class GOSound;

class GOSoundPort {
protected:
  GOSound *m_Sound;
  unsigned m_Index;
  bool m_IsOpen;
  wxString m_Name;
  unsigned m_Channels;
  unsigned m_SamplesPerBuffer;
  unsigned m_SampleRate;
  unsigned m_Latency;
  int m_ActualLatency;

  void SetActualLatency(double latency);
  bool AudioCallback(float *outputBuffer, unsigned int nFrames);

public:
  GOSoundPort(GOSound *sound, wxString name);
  virtual ~GOSoundPort();

  void Init(
    unsigned channels,
    unsigned sample_rate,
    unsigned samples_per_buffer,
    unsigned latency,
    unsigned index);
  virtual void Open() = 0;
  virtual void StartStream() = 0;
  virtual void Close() = 0;

  const wxString &GetName();

  wxString getPortState();
};

#endif
