/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRECORDER_H
#define GOSOUNDRECORDER_H

#include <wx/file.h>
#include <wx/string.h>

#include <atomic>
#include <vector>

#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOMutex.h"

class GOSoundBufferItem;
struct struct_WAVE;

class GOSoundRecorder : public GOSoundWorkItem {
private:
  wxFile m_file;
  GOMutex m_lock;
  GOMutex m_Mutex;
  unsigned m_SampleRate;
  unsigned m_Channels;
  unsigned m_BytesPerSample;
  unsigned m_BufferSize;
  unsigned m_BufferPos;
  unsigned m_SamplesPerBuffer;
  bool m_Recording;
  bool m_Done;
  std::atomic_bool m_Stop;
  std::vector<GOSoundBufferItem *> m_Outputs;
  char *m_Buffer;

  void SetupBuffer();
  template <class T> void ConvertData();
  struct_WAVE generateHeader(unsigned datasize);

public:
  GOSoundRecorder();
  virtual ~GOSoundRecorder();

  void Open(wxString filename);
  bool IsOpen();
  void Close();
  void SetSampleRate(unsigned sample_rate);
  /* 1 = 8 bit, 2 = 16 bit, 3 = 24 bit, 4 = float */
  void SetBytesPerSample(unsigned value);
  void SetOutputs(
    std::vector<GOSoundBufferItem *> outputs, unsigned samples_per_buffer);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();
};

#endif
