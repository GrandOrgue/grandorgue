/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

/**
 * This class is responsible for smoothly changing a volume of samples.
 *
 * There are different types of volumes
 * - Target volume. It is defined by total capacity of the sample size and may
 *   be adjusted in odf and settings.
 * - External volume. It may be changed by an enclosure during playback.
 * - VelocityVolume. It is defined when a key is pressed and may be changed if
 *   another coupled key with the same pipe is pressed with another velocity.
 *   It is considered as part of an
 * - Total volume. It is a production of of all volumes above.
 *
 * There are three stages of playing one sample related to the target volume:
 * - Increasing: from 0 to m_TargetVolume during m_IncreasingFrames. It is used
 *   only for crossfade between two samples.
 * - Constant playing: the volume is not changed. Usually it is used for the
 *   attack sample (because the increasing is already sampled).
 * - Decreasing: from m_СurrentVolume to 0. It is used for both the crossfade
 *   from a loop to a release sample and when playing the release sample when
 *   ReleseTail limitation is used
 *
 * totalVol = targetVolume * externalVolume
 * This volume is applied in the Process() call
 */

class GOSoundFader {
private:
  // The final volume after increasing
  float m_TargetVolume;
  // Additional coeff.
  float m_VelocityVolume;

  // a volume delta for one frame when increasing
  float m_IncreasingDeltaPerFrame;

  // a volume delta for one frame when decreasing
  float m_DecreasingDeltaPerFrame;

  // Last volume points are the volumes at the end of previous Process()
  float m_LastTargetVolumePoint;
  float m_LastExternalVolumePoint;

public:
  /**
   * Setup the fader for constant volume or for increasing from 0 to
   * targetVolume
   * @param targetVolume target volume to reach
   * @param velocityVolume the initial velocity volume. It may be changed later
   * @param nFramesToIncreaseIn if it is 0 then the target volume will be
   *   constant. Else it will smoothly increase from 0 to targetVolume in this
   *   number of frames
   */
  void Setup(
    float targetVolume, float velocityVolume, unsigned nFramesToIncreaseIn = 0);

  /**
   * Start decreasing the volume from the current value
   * (m_LastTargetVolumePoint) to 0
   * @param nFrames number of frames for full decay
   */
  inline void StartDecreasingVolume(unsigned nFrames) {
    // maybe m_TargetVolume has not yet been reached, but the velocity of
    // decreasing should be the same as it has reached
    m_DecreasingDeltaPerFrame = -(m_TargetVolume / nFrames);
  }

  inline float GetVelocityVolume() const { return m_VelocityVolume; }
  inline void SetVelocityVolume(float volume) { m_VelocityVolume = volume; }

  void Process(unsigned nFrames, float *buffer, float externalVolume);

  bool IsSilent() const { return (m_LastTargetVolumePoint <= 0.0f); }
};

#endif /* GOSOUNDFADER_H_ */
