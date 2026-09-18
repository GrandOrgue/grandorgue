/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundRecorderTask.h"

#include <cassert>

#include <wx/intl.h>
#include <wx/log.h>

#include "threading/GOMutexLocker.h"

#include "GOSoundBufferTaskBase.h"

GOSoundRecorderTask::GOSoundRecorderTask()
  : GOSoundTaskBase(PRIORITY_AUDIORECORDER, false),
    m_file(),
    m_lock(),
    m_SampleRate(0),
    m_Channels(2),
    m_BytesPerSample(4),
    m_BufferSize(0),
    m_BufferPos(0),
    m_SamplesPerBuffer(1024),
    m_Recording(false),
    m_Buffer(0) {
  SetupBuffer();
}

GOSoundRecorderTask::~GOSoundRecorderTask() {
  Close();
  if (m_Buffer)
    delete[] m_Buffer;
}

GOSoundRecorderTask::PcmWaveHeader GOSoundRecorderTask::generateHeader(
  unsigned datasize) {
  PcmWaveHeader WAVE = {
    {WAVE_TYPE_RIFF, datasize + 36},
    WAVE_TYPE_WAVE,
    {WAVE_TYPE_FMT, 16},
    {(m_BytesPerSample == 4 ? 3 : 1),
     m_Channels,
     m_SampleRate,
     m_SampleRate * m_BytesPerSample * m_Channels,
     m_BytesPerSample * m_Channels,
     8 * m_BytesPerSample},
    {WAVE_TYPE_DATA, datasize}};
  return WAVE;
}

void GOSoundRecorderTask::Open(wxString filename) {
  PcmWaveHeader WAVE = generateHeader(0);

  Close();

  GOMutexLocker locker(m_lock);

  m_file.Create(filename, true);
  if (!m_file.IsOpened()) {
    wxLogError(_("Unable to open file %s for writing"), filename.c_str());
    return;
  }
  m_file.Write(&WAVE, sizeof(WAVE));

  GOMutexLocker lock(m_mutex);
  m_Recording = true;
  m_BufferPos = 0;
}

bool GOSoundRecorderTask::IsOpen() const { return m_Recording; }

bool GOSoundRecorderTask::IsEmpty() const {
  return !IsOpen() && GOSoundTaskBase::IsEmpty();
}

void GOSoundRecorderTask::Close() {
  GOMutexLocker locker(m_lock);
  {
    GOMutexLocker locker(m_mutex);
    m_Recording = false;
  }
  if (!m_file.IsOpened())
    return;
  PcmWaveHeader WAVE = generateHeader(m_BufferPos);
  m_file.Seek(0);
  m_file.Write(&WAVE, sizeof(WAVE));
  m_file.Flush();
  m_file.Close();
}

void GOSoundRecorderTask::SetSampleRate(unsigned sample_rate) {
  m_SampleRate = sample_rate;
}

void GOSoundRecorderTask::SetBytesPerSample(unsigned value) {
  if (value < 1 || value > 4)
    value = 4;
  m_BytesPerSample = value;
  SetupBuffer();
}

void GOSoundRecorderTask::SetOutputs(
  std::vector<GOSoundBufferTaskBase *> outputs, unsigned samples_per_buffer) {
  m_Outputs = outputs;
  m_SamplesPerBuffer = samples_per_buffer;
  SetupBuffer();
}

void GOSoundRecorderTask::SetupBuffer() {
  // Close() must run before m_Channels is recomputed below: it finalises the
  // WAV header through generateHeader(), which reads m_Channels, so a
  // recording spanning a device reconfiguration must be closed out under the
  // channel count it was recorded with, not the new one.
  Close();
  if (m_Buffer)
    delete[] m_Buffer;
  m_Channels = 0;
  for (unsigned i = 0; i < m_Outputs.size(); i++)
    m_Channels += m_Outputs[i]->GetNChannels();
  m_BufferSize = m_SamplesPerBuffer * m_Channels * m_BytesPerSample;
  m_Buffer = new char[m_BufferSize];
}

static inline int float_to_fixed(float f, unsigned fractional_bits) {
  assert(fractional_bits > 0);
  int max_val = 1 << fractional_bits;
  int f_exp = f * max_val;
  if (f_exp < -max_val)
    return -max_val;
  if (f_exp > max_val - 1)
    return max_val - 1;
  return f_exp;
}

static inline void convertValue(float value, GOInt24LE &result) {
  result = float_to_fixed(value, 23);
}

static inline void convertValue(float value, GOInt16LE &result) {
  result = float_to_fixed(value, 15);
}

static inline void convertValue(float value, GOInt8 &result) {
  result = (unsigned char)(float_to_fixed(value, 7) + 128);
}

static inline void convertValue(float value, float &result) { result = value; }

template <class T> void GOSoundRecorderTask::ConvertData() {
  unsigned start_pos = 0;
  T *buf = (T *)m_Buffer;

  for (unsigned i = 0; i < m_Outputs.size(); i++) {
    GOSoundBufferTaskBase *pOutput = m_Outputs[i];

    pOutput->EnsureBufferReady(m_IsToComplete.load());
    assert(pOutput->GetNFrames() == m_SamplesPerBuffer);

    const unsigned nChannels = pOutput->GetNChannels();

    // gather each output task's channels, which are contiguous, and scatter
    // them into the interleaved WAV frame at the appropriate channel offset;
    // the m_Channels stride skips over the other outputs' channels within
    // each frame, so per-output channel counts may differ
    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      float *pData = pOutput->GetChannelBuffer(channelI).GetData();
      T *pBuf = buf + start_pos + channelI;

      for (unsigned nFramesRest = m_SamplesPerBuffer; nFramesRest;
           nFramesRest--, pData++, pBuf += m_Channels)
        convertValue(*pData, *pBuf);
    }
    start_pos += nChannels;
  }
}

bool GOSoundRecorderTask::DoRun(GOSchedulerThread *pThread) {
  bool isDone = false;

  if (m_Recording) {
    switch (m_BytesPerSample) {
    case 1:
      ConvertData<GOInt8>();
      break;
    case 2:
      ConvertData<GOInt16LE>();
      break;
    case 3:
      ConvertData<GOInt24LE>();
      break;
    case 4:
      ConvertData<float>();
      break;
    }
    m_file.Write(m_Buffer, m_BufferSize);
    m_BufferPos += m_BufferSize;
    isDone = true;
  }

  return isDone;
}

void GOSoundRecorderTask::DiscardContent() {
  Close();
  NewRound();
}
