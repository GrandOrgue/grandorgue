/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundFader.h"

#include <algorithm>

void GOSoundFader::Setup(
  float targetVolume, float velocityVolume, unsigned nFramesToIncreaseIn) {
  m_TargetVolume = targetVolume;

  if (nFramesToIncreaseIn) {
    m_LastTargetVolumePoint = 0.0f;
    m_IncreasingDeltaPerFrame = targetVolume / nFramesToIncreaseIn;
  } else {
    m_LastTargetVolumePoint = targetVolume;
    m_IncreasingDeltaPerFrame = 0.0f;
  }
  m_DecreasingDeltaPerFrame = 0.0f;
  m_VelocityVolume = velocityVolume;
  m_LastExternalVolumePoint = -1; // will be set on the first Process() call
}

// if the external volume is changed, do it smoothly in this number of frames
static constexpr unsigned EXTERNAL_VOLUME_CHANGE_FRAMES = 1024;

void GOSoundFader::Process(
  unsigned nFrames, float *buffer, float externalVolume) {
  // setup process

  // Consider the velocity volume as part of the external volume
  externalVolume *= m_VelocityVolume;

  float startTargetVolumePoint = m_LastTargetVolumePoint;
  // Calculate new m_LastTargetVolumePoint
  // the target volume will be changed from startTargetVolumePoint to
  // m_LastTargetVolumePoint during the nFrames
  float targetVolumeDeltaPerFrame
    = m_IncreasingDeltaPerFrame + m_DecreasingDeltaPerFrame;

  if (targetVolumeDeltaPerFrame != 0.0f) {
    m_LastTargetVolumePoint = std::clamp(
      startTargetVolumePoint + targetVolumeDeltaPerFrame * nFrames,
      0.0f,
      m_TargetVolume);

    if (m_LastTargetVolumePoint >= m_TargetVolume)
      // target volume is reached. Stop increasing
      m_IncreasingDeltaPerFrame = 0.0f;
    else if (m_LastTargetVolumePoint <= 0.0f)
      // Decreasing is finished. Stop it.
      m_DecreasingDeltaPerFrame = 0.0f;
  }

  if (m_LastExternalVolumePoint < 0.0f) // The first Process() call
    m_LastExternalVolumePoint = externalVolume;

  float startExternalVolumePoint = m_LastExternalVolumePoint;
  // Calculate new m_LastExternalVolumePoint
  // the target volume will be changed from startExternalVolumePoint to
  // m_LastExternalVolumePoint during the nFrames period
  if (externalVolume != startExternalVolumePoint)
    m_LastExternalVolumePoint += (externalVolume - startExternalVolumePoint)
      // Assume that external volume is to be reached in MAX_FRAME_SIZE frames
      * std::max(nFrames, EXTERNAL_VOLUME_CHANGE_FRAMES)
      / EXTERNAL_VOLUME_CHANGE_FRAMES;

  float frameTotalVolume = startTargetVolumePoint * startExternalVolumePoint;

  // Process data
  if (
    (m_LastTargetVolumePoint == startTargetVolumePoint)
    && (m_LastExternalVolumePoint == startExternalVolumePoint)) {
    // Adjust the buffer by frameTotalVolume
    for (unsigned int i = 0; i < nFrames; i++, buffer += 2) {
      buffer[0] *= frameTotalVolume;
      buffer[1] *= frameTotalVolume;
    }
  } else {
    // Adjust the buffer smoothly from frameTotalVolume to
    // m_LastTargetVolumePoint * m_LastExternalVolumePoint
    float frameTotalVolumeDelta // changing the volume by one frame
      = (m_LastTargetVolumePoint * m_LastExternalVolumePoint - frameTotalVolume)
      / nFrames;

    for (unsigned int i = 0; i < nFrames; i++, buffer += 2) {
      buffer[0] *= frameTotalVolume;
      buffer[1] *= frameTotalVolume;
      frameTotalVolume += frameTotalVolumeDelta;
    }
  }
}
