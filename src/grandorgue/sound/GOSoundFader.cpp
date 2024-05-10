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
  m_LastTotalVolumePoint = 0.0f;
  m_TargetVolume = target_gain;
  m_IncreasingDeltaPerFrame = target_gain / n_frames;
  m_LastExternalVolumePoint = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::SetupForConstantVolume(float gain) {
  m_IncreasingFrames = 0;
  m_IncreasingDeltaPerFrame = m_DecreasingDeltaPerFrame = 0.0f;
  m_LastTotalVolumePoint = m_TargetVolume = gain;
  m_LastExternalVolumePoint = -1;
  m_VelocityVolume = 1;
}

void GOSoundFader::Process(
  unsigned nFrames, float *buffer, float externalVolume) {
  // setup process

  // Consider the velocity volume as part of the external volume
  externalVolume *= m_VelocityVolume;
  if (m_LastExternalVolumePoint < 0) {
    m_LastExternalVolumePoint = externalVolume;
    m_TotalVolume = m_TargetVolume * externalVolume;
    m_LastTotalVolumePoint *= externalVolume;
  }

  float targetVolumeDeltaPerFrame
    = m_IncreasingDeltaPerFrame + m_DecreasingDeltaPerFrame;
  float frameTotalVolume
    = m_LastTotalVolumePoint;      // the volume for the first frame
  float frameTotalVolumeDelta = 0; // changing the volume by one frame

  if (
    externalVolume != m_LastExternalVolumePoint
    || targetVolumeDeltaPerFrame != 0) {
    /*
     * the volume is changed during the buffer.
     * Calculate frameVolumeDelta and other m_lasTotalVolume
     *
     * Because totalVol = targetVolume * externalVolume,
     * totalVolumeDiff = targetVolumeDiff * externalVolume
     *   + externalVolumeDiff * targetVolume
     */

    float targetVolumeChange = targetVolumeDeltaPerFrame * nFrames;
    // Assume that external volume is fully changed in MAX_FRAME_SIZE frames
    float externalVolumeChange
      = (externalVolume - m_LastExternalVolumePoint) * nFrames / MAX_FRAME_SIZE;
    float targetVolumeDiff = targetVolumeChange * externalVolume;
    float externalVolumeDiff = externalVolumeChange * m_TargetVolume;
    float newLastExternalVolume
      = m_LastExternalVolumePoint + externalVolumeChange;

    m_TotalVolume = m_TargetVolume * newLastExternalVolume;

    float end = m_LastTotalVolumePoint + externalVolumeDiff + targetVolumeDiff;

    if (end < 0) {
      end = 0;
      m_DecreasingDeltaPerFrame = 0;
    } else if (end > m_TotalVolume) {
      end = m_TotalVolume;
      m_IncreasingDeltaPerFrame = 0.0f;
    }
    frameTotalVolumeDelta = (end - m_LastTotalVolumePoint) / (nFrames);
    m_LastExternalVolumePoint = newLastExternalVolume;
    m_LastTotalVolumePoint = end;
  }
  if (m_IncreasingDeltaPerFrame > 0.0f) {
    if (m_IncreasingFrames >= nFrames)
      m_IncreasingFrames -= nFrames;
    else
      m_IncreasingDeltaPerFrame = 0.0f;
  }

  // Procedss data
  if (frameTotalVolumeDelta) {
    for (unsigned int i = 0; i < nFrames; i++, buffer += 2) {
      buffer[0] *= frameTotalVolume;
      buffer[1] *= frameTotalVolume;
      frameTotalVolume += frameTotalVolumeDelta;
    }
  } else {
    for (unsigned int i = 0; i < nFrames; i++, buffer += 2) {
      buffer[0] *= frameTotalVolume;
      buffer[1] *= frameTotalVolume;
    }
  }
}
