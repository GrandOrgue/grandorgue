/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCONVERSIONS_H
#define GOSOUNDCONVERSIONS_H

#include <cassert>
#include <climits>
#include <cstdint>

/**
 * Conversions between the units used by the sound engine and the organ
 * model: decibels, cents, milliseconds and frames. A stateless DSP kernel.
 */
class GOSoundConversions {
public:
  /** The gain that stands for silence, in dB */
  static constexpr float MUTE_GAIN_DB = -121.0f;
  /** The lowest gain that is not silence, in dB */
  static constexpr float MIN_GAIN_DB = -120.0f;
  /** The default gain of an audio output, in dB: the volume is unchanged */
  static constexpr float DEFAULT_GAIN_DB = 0.0f;
  /** The highest gain, in dB */
  static constexpr float MAX_GAIN_DB = 40.0f;

  /** The number of cents in an octave */
  static constexpr double CENTS_PER_OCTAVE = 1200.0;

private:
  /* The addends of the integer rounding of x / 1000 */
  static constexpr unsigned ROUND_NEAREST_ADDEND = 500;
  static constexpr unsigned ROUND_UP_ADDEND = 999;

  /**
   * The common part of msToFramesRounded() and msToFramesCeil().
   * @param ms the duration in milliseconds
   * @param sampleRate the sample rate in Hz, > 0
   * @param roundingAddend the rounding of the division by 1000: 500 rounds to
   *   the nearest, 999 up
   * @return the number of frames; it must fit into unsigned
   */
  static inline unsigned msToFramesWithAddend(
    unsigned ms, unsigned sampleRate, unsigned roundingAddend) {
    assert(sampleRate > 0);
    assert(roundingAddend < 1000);

    const uint64_t nFrames
      = ((uint64_t)ms * sampleRate + roundingAddend) / 1000;

    assert(nFrames <= UINT_MAX);
    return (unsigned)nFrames;
  }

public:
  /**
   * Converts a gain in dB to the linear factor of the amplitude: 10^(dB/20).
   * 0 dB gives exactly 1. It applies no limits: see gainToVolumeOrMute().
   * @param gainDb the gain in dB, finite
   * @return the factor of the amplitude, > 0
   */
  static float gainToVolume(float gainDb);

  /**
   * The same as gainToVolume(), but a gain outside MIN_GAIN_DB..MAX_GAIN_DB
   * (including MUTE_GAIN_DB) means silence.
   * @param gainDb the gain in dB, finite
   * @return the factor of the amplitude, or 0 for a muted gain
   */
  static float gainToVolumeOrMute(float gainDb);

  /**
   * Converts a pitch shift in cents to the ratio of the frequencies:
   * 2^(cents/1200). 0 cents gives exactly 1, 1200 cents gives 2, a negative
   * shift gives a ratio below 1.
   * @param cents the pitch shift in cents, finite
   * @return the ratio of the shifted frequency to the original one, > 0
   */
  static double centsToRatio(double cents);

  /**
   * Converts a ratio of two frequencies to the pitch shift in cents:
   * 1200 * log2(ratio). The inverse of centsToRatio().
   * @param ratio the ratio of the shifted frequency to the original one,
   *   finite and > 0
   * @return the pitch shift in cents: 0 for 1, 1200 for 2, negative below 1
   */
  static double ratioToCents(double ratio);

  /**
   * Converts a duration in milliseconds to frames, rounded to the nearest
   * frame (halves up).
   * @param ms the duration in milliseconds
   * @param sampleRate the sample rate in Hz, > 0
   * @return the number of frames; it must fit into unsigned
   */
  static inline unsigned msToFramesRounded(unsigned ms, unsigned sampleRate) {
    return msToFramesWithAddend(ms, sampleRate, ROUND_NEAREST_ADDEND);
  }

  /**
   * Converts a duration in milliseconds to frames, rounded up: the frames
   * never cover less than the duration.
   * @param ms the duration in milliseconds
   * @param sampleRate the sample rate in Hz, > 0
   * @return the number of frames; it must fit into unsigned
   */
  static inline unsigned msToFramesCeil(unsigned ms, unsigned sampleRate) {
    return msToFramesWithAddend(ms, sampleRate, ROUND_UP_ADDEND);
  }

  /**
   * Converts a number of frames to milliseconds, rounded down.
   * @param nFrames the number of frames; nFrames * 1000 must fit into
   *   uint64_t
   * @param sampleRate the sample rate in Hz, > 0
   * @return the duration in milliseconds
   */
  static inline uint64_t framesToMs(uint64_t nFrames, unsigned sampleRate) {
    assert(sampleRate > 0);
    assert(nFrames <= UINT64_MAX / 1000);
    return nFrames * 1000 / sampleRate;
  }
};

#endif /* GOSOUNDCONVERSIONS_H */
