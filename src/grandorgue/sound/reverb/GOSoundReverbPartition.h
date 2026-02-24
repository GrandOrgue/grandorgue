/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDREVERBPARTITION_H
#define GOSOUNDREVERBPARTITION_H

#include "fftw3.h"
#include "ptrvector.h"

class GOSoundReverbPartition {
private:
  unsigned m_PartitionSize;
  unsigned m_PartitionCount;
  float *m_fftwTmpReal;
  fftwf_complex *m_fftwTmpComplex;
  fftwf_plan m_TimeToFreq;
  fftwf_plan m_FreqToTime;
  float *m_Input;
  float *m_Output;
  unsigned m_InputPos;
  unsigned m_InputStartPos;
  unsigned m_OutputPos;
  ptr_vector<fftwf_complex> m_InputHistory;
  ptr_vector<fftwf_complex> m_IRData;
  unsigned m_InputHistoryPos;

  void ZeroComplex(fftwf_complex *data, unsigned len);

public:
  GOSoundReverbPartition(unsigned size, unsigned cnt, unsigned start_pos);
  ~GOSoundReverbPartition();

  void Reset();
  void Process(float *output_buf, const float *input_buf, unsigned len);
  void AddIR(const float *data, unsigned pos, unsigned len, unsigned offset);
  unsigned GetLength();
};

#endif
