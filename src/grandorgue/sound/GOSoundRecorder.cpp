/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundRecorder.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOSoundBufferItem.h"
#include "GOWaveTypes.h"
#include "threading/GOMutexLocker.h"

#pragma pack(push, 1)

struct struct_WAVE {
  GO_WAVECHUNKHEADER riffHeader;
  GO_WAVETYPEFIELD riffIdent;
  GO_WAVECHUNKHEADER formatHeader;
  GO_WAVEFORMATPCM formatBlock;
  GO_WAVECHUNKHEADER dataHeader;
};

#pragma pack(pop)

GOSoundRecorder::GOSoundRecorder()
  : m_file(),
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

GOSoundRecorder::~GOSoundRecorder() {
  Close();
  if (m_Buffer)
    delete[] m_Buffer;
}

struct_WAVE GOSoundRecorder::generateHeader(unsigned datasize) {
  struct_WAVE WAVE = {
    {WAVE_TYPE_RIFF, datasize + 36},
    WAVE_TYPE_WAVE,
    {WAVE_TYPE_FMT, 16},
    {m_BytesPerSample == 4 ? 3 : 1,
     m_Channels,
     m_SampleRate,
     m_SampleRate * m_BytesPerSample * m_Channels,
     m_BytesPerSample * m_Channels,
     8 * m_BytesPerSample},
    {WAVE_TYPE_DATA, datasize}};
  return WAVE;
}

void GOSoundRecorder::Open(wxString filename) {
  struct_WAVE WAVE = generateHeader(0);

  Close();

  GOMutexLocker locker(m_lock);

  m_file.Create(filename, true);
  if (!m_file.IsOpened()) {
    wxLogError(_("Unable to open file %s for writing"), filename.c_str());
    return;
  }
  m_file.Write(&WAVE, sizeof(WAVE));

  GOMutexLocker lock(m_Mutex);
  m_Recording = true;
  m_BufferPos = 0;
}

bool GOSoundRecorder::IsOpen() { return m_Recording; }

void GOSoundRecorder::Close() {
  GOMutexLocker locker(m_lock);
  {
    GOMutexLocker locker(m_Mutex);
    m_Recording = false;
  }
  if (!m_file.IsOpened())
    return;
  struct_WAVE WAVE = generateHeader(m_BufferPos);
  m_file.Seek(0);
  m_file.Write(&WAVE, sizeof(WAVE));
  m_file.Flush();
  m_file.Close();
}

void GOSoundRecorder::SetSampleRate(unsigned sample_rate) {
  m_SampleRate = sample_rate;
}

void GOSoundRecorder::SetBytesPerSample(unsigned value) {
  if (value < 1 || value > 4)
    value = 4;
  m_BytesPerSample = value;
  SetupBuffer();
}

void GOSoundRecorder::SetOutputs(
  std::vector<GOSoundBufferItem *> outputs, unsigned samples_per_buffer) {
  m_Outputs = outputs;
  m_SamplesPerBuffer = samples_per_buffer;
  SetupBuffer();
}

void GOSoundRecorder::SetupBuffer() {
  Close();
  if (m_Buffer)
    delete[] m_Buffer;
  m_Channels = 0;
  for (unsigned i = 0; i < m_Outputs.size(); i++)
    m_Channels += m_Outputs[i]->GetChannels();
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

static void convertValue(float value, GOInt24LE &result) {
  result = float_to_fixed(value, 23);
}

static void convertValue(float value, GOInt16LE &result) {
  result = float_to_fixed(value, 15);
}

static void convertValue(float value, GOInt8 &result) {
  result = (unsigned char)(float_to_fixed(value, 7) + 128);
}

static void convertValue(float value, float &result) { result = value; }

template <class T> void GOSoundRecorder::ConvertData() {
  unsigned start_pos = 0;
  T *buf = (T *)m_Buffer;
  for (unsigned i = 0; i < m_Outputs.size(); i++) {
    m_Outputs[i]->Finish(m_Stop.load());

    unsigned pos = start_pos;
    unsigned inc = m_Channels - m_Outputs[i]->GetChannels();
    for (unsigned l = 0, j = 0; j < m_SamplesPerBuffer; j++) {
      for (unsigned k = 0; k < m_Outputs[i]->GetChannels(); k++, l++) {
        convertValue(m_Outputs[i]->m_Buffer[l], buf[pos++]);
      }
      pos += inc;
    }
    start_pos += m_Outputs[i]->GetChannels();
  }
}

unsigned GOSoundRecorder::GetGroup() { return AUDIORECORDER; }

unsigned GOSoundRecorder::GetCost() { return 0; }

bool GOSoundRecorder::GetRepeat() { return false; }

void GOSoundRecorder::Run(GOSoundThread *thread) {
  if (!m_Recording)
    return;
  if (m_Done)
    return;
  GOMutexLocker locker(m_Mutex);
  if (m_Done)
    return;
  if (!m_Recording)
    return;

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
  m_Done = true;
}

void GOSoundRecorder::Exec() {
  m_Stop.store(true);
  Run();
}

void GOSoundRecorder::Clear() {
  Close();
  Reset();
}

void GOSoundRecorder::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Done = false;
  m_Stop.store(false);
}
