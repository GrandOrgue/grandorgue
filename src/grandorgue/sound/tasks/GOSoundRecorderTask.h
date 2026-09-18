/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRECORDERTASK_H
#define GOSOUNDRECORDERTASK_H

#include <vector>

#include <wx/file.h>
#include <wx/string.h>

#include "threading/GOMutex.h"

#include "GOSoundTaskBase.h"
#include "GOWaveTypes.h"

class GOSchedulerThread;
class GOSoundBufferTaskBase;

class GOSoundRecorderTask : public GOSoundTaskBase {
public:
#pragma pack(push, 1)
  /** The canonical 44-byte PCM WAV header this task writes: RIFF chunk, WAVE
   * type, fmt chunk, data chunk header (no sample data). */
  struct PcmWaveHeader {
    GO_WAVECHUNKHEADER riffHeader;
    GO_WAVETYPEFIELD riffIdent;
    GO_WAVECHUNKHEADER formatHeader;
    GO_WAVEFORMATPCM formatBlock;
    GO_WAVECHUNKHEADER dataHeader;
  };
#pragma pack(pop)

  /** Size, in bytes, of the WAV header written by Open()/Close() before the
   * sample data - exposed so tests can locate the data section without
   * hardcoding the value. */
  static constexpr unsigned WAV_HEADER_SIZE = sizeof(PcmWaveHeader);

private:
  wxFile m_file;
  GOMutex m_lock;
  unsigned m_SampleRate;
  unsigned m_Channels;
  unsigned m_BytesPerSample;
  unsigned m_BufferSize;
  unsigned m_BufferPos;
  unsigned m_SamplesPerBuffer;
  bool m_Recording;
  std::vector<GOSoundBufferTaskBase *> m_Outputs;
  char *m_Buffer;

  void SetupBuffer();
  template <class T> void ConvertData();
  PcmWaveHeader generateHeader(unsigned datasize);

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  GOSoundRecorderTask();
  virtual ~GOSoundRecorderTask();

  bool IsEmpty() const override;

  void Open(wxString filename);
  bool IsOpen() const;
  void Close();
  void SetSampleRate(unsigned sample_rate);
  /* 1 = 8 bit, 2 = 16 bit, 3 = 24 bit, 4 = float */
  void SetBytesPerSample(unsigned value);
  unsigned GetBytesPerSample() const { return m_BytesPerSample; }
  void SetOutputs(
    std::vector<GOSoundBufferTaskBase *> outputs, unsigned samples_per_buffer);

  void DiscardContent() override;
};

#endif
