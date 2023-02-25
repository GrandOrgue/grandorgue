/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERITEM_H
#define GOSOUNDBUFFERITEM_H

class GOSoundThread;

class GOSoundBufferItem {
protected:
  unsigned m_SamplesPerBuffer;
  unsigned m_Channels;

public:
  GOSoundBufferItem(unsigned samples_per_buffer, unsigned channels)
    : m_SamplesPerBuffer(samples_per_buffer), m_Channels(channels) {
    m_Buffer = new float[m_SamplesPerBuffer * m_Channels];
  }
  virtual ~GOSoundBufferItem() { delete[] m_Buffer; }

  virtual void Finish(bool stop, GOSoundThread *pThread = nullptr) = 0;

  float *m_Buffer;

  unsigned GetSamplesPerBuffer() { return m_SamplesPerBuffer; }

  unsigned GetChannels() { return m_Channels; }
};

#endif
