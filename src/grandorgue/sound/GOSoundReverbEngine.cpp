/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReverbEngine.h"

#include <algorithm>

#include "GOSoundReverbPartition.h"

GOSoundReverbEngine::GOSoundReverbEngine(unsigned samples_per_buffer)
  : m_Partitions() {
  if (samples_per_buffer < 128) {
    m_Partitions.push_back(new GOSoundReverbPartition(64, 3, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(128, 6, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(512, 6, 64));
    m_Partitions.push_back(new GOSoundReverbPartition(2048, 6, 64 + 256));
    m_Partitions.push_back(
      new GOSoundReverbPartition(8192, 121, 64 + 256 + 1024));
  } else if (samples_per_buffer < 256) {
    m_Partitions.push_back(new GOSoundReverbPartition(128, 7, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(512, 6, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(2048, 6, 256));
    m_Partitions.push_back(new GOSoundReverbPartition(8192, 121, 256 + 1024));
  } else if (samples_per_buffer < 512) {
    m_Partitions.push_back(new GOSoundReverbPartition(256, 3, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(512, 6, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(2048, 6, 256));
    m_Partitions.push_back(new GOSoundReverbPartition(8192, 121, 256 + 1024));
  } else if (samples_per_buffer < 1024) {
    m_Partitions.push_back(new GOSoundReverbPartition(512, 7, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(2048, 6, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(8192, 121, 1024));
  } else {
    m_Partitions.push_back(new GOSoundReverbPartition(1024, 3, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(2048, 6, 0));
    m_Partitions.push_back(new GOSoundReverbPartition(8192, 121, 1024));
  }
}

GOSoundReverbEngine::~GOSoundReverbEngine() {}

void GOSoundReverbEngine::Reset() {
  for (unsigned i = 0; i < m_Partitions.size(); i++)
    m_Partitions[i]->Reset();
}

void GOSoundReverbEngine::Process(
  float *output_buf, const float *input_buf, unsigned len) {
  std::fill(output_buf, output_buf + len, 0);
  for (unsigned i = 0; i < m_Partitions.size(); i++)
    m_Partitions[i]->Process(output_buf, input_buf, len);
}

void GOSoundReverbEngine::AddIR(const float *data, unsigned pos, unsigned len) {
  unsigned offset = 0;
  for (unsigned i = 0; i < m_Partitions.size(); i++) {
    m_Partitions[i]->AddIR(data, pos, len, offset);
    offset += m_Partitions[i]->GetLength();
  }
}
