/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSampleStatistic.h"

#include <math.h>

GOSampleStatistic::GOSampleStatistic() : m_Valid(false) {}

GOSampleStatistic::~GOSampleStatistic() {}

void GOSampleStatistic::Prepare() {
  if (!m_Valid)
    Init();
}

void GOSampleStatistic::Init() {
  m_Valid = true;
  m_MemorySize = 0;
  m_EndSegmentSize = 0;
  m_MinBitsPerSample = 0xff;
  m_MaxBitsPerSample = 0;
  m_UsedBits = 0;
  m_AllocatedSamples = 0;
}

void GOSampleStatistic::Cumulate(const GOSampleStatistic &stat) {
  if (!stat.IsValid())
    return;
  Prepare();
  m_MemorySize += stat.m_MemorySize;
  m_EndSegmentSize += stat.m_EndSegmentSize;
  if (m_MinBitsPerSample > stat.m_MinBitsPerSample)
    m_MinBitsPerSample = stat.m_MinBitsPerSample;
  if (m_MaxBitsPerSample < stat.m_MaxBitsPerSample)
    m_MaxBitsPerSample = stat.m_MaxBitsPerSample;
  m_UsedBits += stat.m_UsedBits;
  m_AllocatedSamples += stat.m_AllocatedSamples;
}

bool GOSampleStatistic::IsValid() const { return m_Valid; }

void GOSampleStatistic::SetMemorySize(size_t size) {
  Prepare();
  m_MemorySize = size;
}

size_t GOSampleStatistic::GetMemorySize() const { return m_MemorySize; }

void GOSampleStatistic::SetEndSegmentSize(size_t size) {
  Prepare();
  m_EndSegmentSize = size;
}

void GOSampleStatistic::SetBitsPerSample(
  unsigned bits, unsigned samples, unsigned max_value) {
  if (bits < m_MinBitsPerSample)
    m_MinBitsPerSample = bits;
  if (bits > m_MaxBitsPerSample)
    m_MaxBitsPerSample = bits;
  m_UsedBits = samples * (size_t)max_value;
  m_AllocatedSamples = samples;
}

size_t GOSampleStatistic::GetEndSegmentSize() const { return m_EndSegmentSize; }

unsigned GOSampleStatistic::GetMinBitPerSample() const {
  return m_MinBitsPerSample;
}

unsigned GOSampleStatistic::GetMaxBitPerSample() const {
  return m_MaxBitsPerSample;
}

float GOSampleStatistic::GetUsedBits() const {
  if (m_AllocatedSamples)
    return 1 + log2(m_UsedBits / (double)m_AllocatedSamples);
  else
    return 0;
}
