/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDREVERBENGINE_H
#define GOSOUNDREVERBENGINE_H

#include "ptrvector.h"

class GOSoundReverbPartition;

class GOSoundReverbEngine {
private:
  ptr_vector<GOSoundReverbPartition> m_Partitions;

public:
  GOSoundReverbEngine(unsigned samples_per_buffer);
  ~GOSoundReverbEngine();

  void Reset();
  void Process(float *output_buf, const float *input_buf, unsigned len);
  void AddIR(const float *data, unsigned pos, unsigned len);
};

#endif
