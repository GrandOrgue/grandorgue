/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundFader.h"

#include "GOSoundDefs.h"

void GOSoundFader::NewAttacking(float target_gain, unsigned n_frames) {
  m_nb_attack_frames_left = n_frames;
  m_decay = 0.0f;
  m_gain = 0.0f;
  m_target = target_gain;
  m_attack = target_gain / n_frames;
  m_last_volume = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::NewConstant(float gain) {
  m_nb_attack_frames_left = 0;
  m_attack = m_decay = 0.0f;
  m_gain = m_target = gain;
  m_last_volume = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::Process(unsigned n_blocks, float *buffer, float volume) {
  // setup process
  volume *= m_VelocityVolume;
  if (m_last_volume < 0) {
    m_last_volume = volume;
    m_real_target = m_target * volume;
    m_gain *= volume;
  }

  float gain = m_gain;
  float gain_delta = 0;

  if (volume != m_last_volume || m_attack + m_decay != 0) {
    float volume_diff
      = m_target * (volume - m_last_volume) * n_blocks / MAX_FRAME_SIZE;
    float fade_diff = n_blocks * (m_attack + m_decay) * volume;
    float new_last_volume
      = m_last_volume + ((volume - m_last_volume) * n_blocks) / MAX_FRAME_SIZE;
    m_real_target = m_target * new_last_volume;

    float end = m_gain + volume_diff + fade_diff;
    if (end < 0) {
      end = 0;
      m_decay = 0;
    } else if (end > m_real_target) {
      end = m_real_target;
      m_attack = 0.0f;
    }
    gain_delta = (end - m_gain) / (n_blocks);
    m_last_volume = new_last_volume;
    m_gain = end;
  }
  if (m_attack > 0.0f) {
    if (m_nb_attack_frames_left >= n_blocks)
      m_nb_attack_frames_left -= n_blocks;
    else
      m_attack = 0.0f;
  }

  // Procedss data
  if (gain_delta) {
    for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
      buffer[0] *= gain;
      buffer[1] *= gain;
      gain += gain_delta;
    }
  } else {
    for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
      buffer[0] *= gain;
      buffer[1] *= gain;
    }
  }
}
