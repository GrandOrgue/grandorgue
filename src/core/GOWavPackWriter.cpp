/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOWavPackWriter.h"

GOWavPackWriter::GOWavPackWriter() : m_Output(), m_Context(0) {}

GOWavPackWriter::~GOWavPackWriter() { Close(); }

bool GOWavPackWriter::Write(void *data, int32_t count) {
  if (count < 0)
    return false;
  m_Output.Append((const uint8_t *)data, count);
  return true;
}

int GOWavPackWriter::WriteCallback(void *id, void *data, int32_t bcount) {
  if (id == NULL)
    return 0;
  return ((GOWavPackWriter *)id)->Write(data, bcount) ? 1 : 0;
}

bool GOWavPackWriter::Init(
  unsigned channels,
  unsigned bitsPerSample,
  unsigned bytesPerSample,
  unsigned sampleRate,
  unsigned sampleCount) {
  Close();
  m_Output.free();
  m_Context = WavpackOpenFileOutput(&WriteCallback, this, NULL);
  if (!m_Context)
    return false;
  WavpackConfig config;
  memset(&config, 0, sizeof(config));
  config.bits_per_sample = bitsPerSample;
  config.bytes_per_sample = bytesPerSample;
  config.sample_rate = sampleRate;
  config.num_channels = channels;
  config.channel_mask = channels == 1 ? 4 : 3;
  config.flags = CONFIG_VERY_HIGH_FLAG | CONFIG_EXTRA_MODE;
  config.xmode = 6;
  config.float_norm_exp = bitsPerSample == 4 ? 127 : 0;
  return WavpackSetConfiguration(m_Context, &config, sampleCount) != 0;
}

bool GOWavPackWriter::AddWrapper(GOBuffer<uint8_t> &header) {
  return WavpackAddWrapper(m_Context, header.get(), header.GetSize()) != 0;
}

bool GOWavPackWriter::AddSampleData(GOBuffer<int32_t> &sampleData) {
  if (WavpackPackInit(m_Context) == 0)
    return false;
  return WavpackPackSamples(
           m_Context,
           sampleData.get(),
           sampleData.GetCount() / WavpackGetNumChannels(m_Context))
    != 0;
}

bool GOWavPackWriter::Close() {
  if (!m_Context)
    return false;
  m_Context = WavpackCloseFile(m_Context);
  return m_Context == NULL;
}

bool GOWavPackWriter::GetResult(GOBuffer<uint8_t> &result) {
  if (WavpackFlushSamples(m_Context) == 0)
    return false;
  if (!Close())
    return false;
  result = std::move(m_Output);
  return true;
}
