/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundPort.h"

#include <wx/intl.h>
#include <wx/thread.h>

#include "sound/GOSound.h"

GOSoundPort::GOSoundPort(GOSound *sound, wxString name)
  : m_Sound(sound),
    m_Index(0),
    m_IsOpen(false),
    m_Name(name),
    m_Channels(0),
    m_SamplesPerBuffer(0),
    m_SampleRate(0),
    m_Latency(0),
    m_ActualLatency(-1) {}

GOSoundPort::~GOSoundPort() {}

void GOSoundPort::Init(
  unsigned channels,
  unsigned sample_rate,
  unsigned samples_per_buffer,
  unsigned latency,
  unsigned index) {
  m_Index = index;
  m_Channels = channels;
  m_SampleRate = sample_rate;
  m_SamplesPerBuffer = samples_per_buffer;
  m_Latency = latency;
}

void GOSoundPort::SetActualLatency(double latency) {
  if (latency < m_SamplesPerBuffer / (double)m_SampleRate)
    latency = m_SamplesPerBuffer / (double)m_SampleRate;
  if (latency < 2 * m_SamplesPerBuffer / (double)m_SampleRate)
    latency += m_SamplesPerBuffer / (double)m_SampleRate;
  m_ActualLatency = latency * 1000;
}

bool GOSoundPort::AudioCallback(float *outputBuffer, unsigned int nFrames) {
  return m_Sound->AudioCallback(m_Index, outputBuffer, nFrames);
}

const wxString &GOSoundPort::GetName() { return m_Name; }
wxString GOSoundPort::getPortState() {
  if (m_ActualLatency < 0)
    return wxString::Format(_("%s: unknown"), GetName().c_str());
  else
    return wxString::Format(_("%s: %d ms"), GetName().c_str(), m_ActualLatency);
}
