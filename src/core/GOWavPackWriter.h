/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOWAVPACKWRITER_H
#define GOWAVPACKWRITER_H

#include <wavpack/wavpack.h>

#include "GOBuffer.h"

class GOWavPackWriter {
private:
  GOBuffer<uint8_t> m_Output;
  WavpackContext *m_Context;

  bool Write(void *data, int32_t count);
  static int WriteCallback(void *id, void *data, int32_t bcount);
  bool Close();

public:
  GOWavPackWriter();
  ~GOWavPackWriter();

  bool Init(
    unsigned channels,
    unsigned bitsPerSample,
    unsigned bytesPerSample,
    unsigned sampleRate,
    unsigned sampleCount);
  bool AddWrapper(GOBuffer<uint8_t> &header);
  bool AddSampleData(GOBuffer<int32_t> &sampleData);
  bool GetResult(GOBuffer<uint8_t> &result);
};

#endif
