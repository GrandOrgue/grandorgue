/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
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

  float startTargetVolumePoint = m_LastTargetVolumePoint;

  // Calculate new m_LastTargetVolumePoint
  // the target volume will be changed from startTargetVolumePoint to
  // m_LastTargetVolumePoint during the nFrames

  unsigned framesLeftAfterIncreasing = nFrames;

  // increasing section
  if (m_IncreasingDeltaPerFrame > 0.0f) {
    // we are increasing now
    float newTargetVolumePoint
      = m_LastTargetVolumePoint + m_IncreasingDeltaPerFrame * nFrames;

    if (newTargetVolumePoint < m_TargetVolume) {
      framesLeftAfterIncreasing = 0; // we are increase during the whole period
      m_LastTargetVolumePoint = newTargetVolumePoint;
    } else {
      // we reach m_TargetVolume inside this nFrames period

      // calculate how many frames left after reaching m_TargetVolume
      framesLeftAfterIncreasing = nFrames
        - unsigned((m_TargetVolume - m_LastTargetVolumePoint)
                   / m_IncreasingDeltaPerFrame);
      // stop increasing
      m_IncreasingDeltaPerFrame = 0.0f;
      m_LastTargetVolumePoint = m_TargetVolume;
    }
  }

  // decreasing section
  if (framesLeftAfterIncreasing && m_DecreasingDeltaPerFrame > 0.0f) {
    // decrease the volume
    float newTargetVolumePoint = m_LastTargetVolumePoint
      - m_DecreasingDeltaPerFrame * framesLeftAfterIncreasing;

    if (newTargetVolumePoint > 0.0f) {
      m_LastTargetVolumePoint = newTargetVolumePoint;
    } else {
      // stop decreasing
      m_DecreasingDeltaPerFrame = 0.0f;
      m_LastTargetVolumePoint = 0.0f;
    }
  }

  // Calculate the external volume
  float targetExternalVolume = m_VelocityVolume * externalVolume;

  if (m_LastExternalVolumePoint < 0.0f)
    m_LastExternalVolumePoint = targetExternalVolume;

  float startExternalVolumePoint = m_LastExternalVolumePoint;

  // Calculate new m_LastExternalVolumePoint
  // the target volume will be changed from startExternalVolumePoint to
  // m_LastExternalVolumePoint during the nFrames period
  if (targetExternalVolume != startExternalVolumePoint)
    m_LastExternalVolumePoint
      += (targetExternalVolume - startExternalVolumePoint)
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
