/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundPlayingFader.h"

#include <algorithm>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"

void GOSoundPlayingFader::Setup(
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

void GOSoundPlayingFader::Process(
  float externalAmplitude, GOSoundBufferMutable &outBuffer) {
  float *pData = outBuffer.GetData();
  const unsigned nFrames = outBuffer.GetNFrames();

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
  float targetExternalVolume = m_VelocityVolume * externalAmplitude;

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

  // Process data: one linear ramp of the total volume over the whole period.
  // If the total volume does not change, the GainRamp constructor makes a
  // constant-gain ramp (0 frames): fade() then only dispatches by the gain
  // (1: nothing, 0: fill, otherwise multiply)
  const float startVolume = startTargetVolumePoint * startExternalVolumePoint;
  const float endVolume = m_LastTargetVolumePoint * m_LastExternalVolumePoint;

  GOSoundLinearFaderCore::fade<2>(
    GOSoundLinearFaderCore::GainRamp(startVolume, endVolume, nFrames),
    0,
    nFrames,
    pData);
}
