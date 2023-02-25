/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSAMPLESTATISTIC_H
#define GOSAMPLESTATISTIC_H

#include <stddef.h>

class GOSampleStatistic {
private:
  bool m_Valid;
  size_t m_MemorySize;
  size_t m_EndSegmentSize;
  unsigned m_MinBitsPerSample;
  unsigned m_MaxBitsPerSample;
  size_t m_AllocatedSamples;
  size_t m_UsedBits;

  void Prepare();

public:
  GOSampleStatistic();
  ~GOSampleStatistic();

  void Init();
  void Cumulate(const GOSampleStatistic &stat);

  void SetMemorySize(size_t size);
  void SetEndSegmentSize(size_t size);
  void SetBitsPerSample(unsigned bits, unsigned samples, unsigned max_value);

  bool IsValid() const;
  size_t GetMemorySize() const;
  size_t GetEndSegmentSize() const;
  unsigned GetMinBitPerSample() const;
  unsigned GetMaxBitPerSample() const;
  float GetUsedBits() const;
};

#endif
