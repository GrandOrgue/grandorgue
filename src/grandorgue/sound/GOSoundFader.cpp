/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundFader.h"

#include "GOSoundDefs.h"

void GOSoundFader::SetupForIncreasingVolume(
  float target_gain, unsigned n_frames) {
  m_IncreasingFrames = n_frames;
  m_DecreasingDeltaPerFrame = 0.0f;
  m_LastTotalVolume = 0.0f;
  m_TargetVolume = target_gain;
  m_IncreasingDeltaPerFrame = target_gain / n_frames;
  m_LastExternalVolume = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::SetupForConstantVolume(float gain) {
  m_IncreasingFrames = 0;
  m_IncreasingDeltaPerFrame = m_DecreasingDeltaPerFrame = 0.0f;
  m_LastTotalVolume = m_TargetVolume = gain;
  m_LastExternalVolume = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::Process(
  unsigned n_blocks, float *buffer, float externalVolume) {
  // setup process
  externalVolume *= m_VelocityVolume;
  if (m_LastExternalVolume < 0) {
    m_LastExternalVolume = externalVolume;
    m_TotalVolume = m_TargetVolume * externalVolume;
    m_LastTotalVolume *= externalVolume;
  }

  float frameVolume = m_LastTotalVolume; // the volume for the first frame
  float frameVolumeDelta = 0;            // changing the volume by one frame

  if (
    externalVolume != m_LastExternalVolume
    || m_IncreasingDeltaPerFrame + m_DecreasingDeltaPerFrame != 0) {
    /*
     * the volume is changed during the buffer.
     * Calculate frameVolumeDelta and other m_lasTotalVolume
     *
     * Because totalVol = targetVolume * externalVolume,
     * totalVolumeDiff = targetVolumeDiff * externalVolume
     *   + externalVolumeDiff * targetVolume
     */

    // Assume that external volume is fully changed in MAX_FRAME_SIZE frames
    float externalVolumeDiff = m_TargetVolume
      * (externalVolume - m_LastExternalVolume) * n_blocks / MAX_FRAME_SIZE;
    float fade_diff = n_blocks
      * (m_IncreasingDeltaPerFrame + m_DecreasingDeltaPerFrame)
      * externalVolume;
    float newLastExternalVolume = m_LastExternalVolume
      + ((externalVolume - m_LastExternalVolume) * n_blocks) / MAX_FRAME_SIZE;
    m_TotalVolume = m_TargetVolume * newLastExternalVolume;

    float end = m_LastTotalVolume + externalVolumeDiff + fade_diff;

    if (end < 0) {
      end = 0;
      m_DecreasingDeltaPerFrame = 0;
    } else if (end > m_TotalVolume) {
      end = m_TotalVolume;
      m_IncreasingDeltaPerFrame = 0.0f;
    }
    frameVolumeDelta = (end - m_LastTotalVolume) / (n_blocks);
    m_LastExternalVolume = newLastExternalVolume;
    m_LastTotalVolume = end;
  }
  if (m_IncreasingDeltaPerFrame > 0.0f) {
    if (m_IncreasingFrames >= n_blocks)
      m_IncreasingFrames -= n_blocks;
    else
      m_IncreasingDeltaPerFrame = 0.0f;
  }

  // Procedss data
  if (frameVolumeDelta) {
    for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
      buffer[0] *= frameVolume;
      buffer[1] *= frameVolume;
      frameVolume += frameVolumeDelta;
    }
  } else {
    for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
      buffer[0] *= frameVolume;
      buffer[1] *= frameVolume;
    }
  }
}
