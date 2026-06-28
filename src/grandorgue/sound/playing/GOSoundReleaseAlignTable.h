/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRELEASEALIGNTABLE_H
#define GOSOUNDRELEASEALIGNTABLE_H

#include "GOSoundAudioSection.h"

class GOCache;
class GOCacheWriter;
class GOTestReleaseAlignTable;

#define PHASE_ALIGN_DERIVATIVES 2
#define PHASE_ALIGN_AMPLITUDES 32
#define PHASE_ALIGN_MIN_FREQUENCY 20 /* Hertz */

class GOSoundReleaseAlignTable {
  friend class GOTestReleaseAlignTable;

private:
  unsigned m_PhaseAlignMaxAmplitude;
  unsigned m_PhaseAlignMaxDerivative;
  /* Sample-position lookup table indexed by (derivative bucket, amplitude
   * bucket). Values are absolute sample offsets, always non-negative. */
  unsigned m_PositionEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  /**
   * Maps a shifted value into a bucket index in [0, nBuckets).
   * Clamps shiftedValue to [0, 2*maxValue) before dividing, so the result
   * is always in range. Returns the midpoint bucket when maxValue is zero.
   *
   * @param shiftedValue  Value shifted by (maxValue - 1), may be out of range.
   * @param maxValue      Half-width of the symmetric range; zero means unknown.
   * @param nBuckets      Number of buckets (PHASE_ALIGN_DERIVATIVES or
   *                      PHASE_ALIGN_AMPLITUDES).
   * @return              Index in [0, nBuckets).
   */
  static unsigned computeBucketIndex(
    int shiftedValue, unsigned maxValue, unsigned nBuckets);

public:
  GOSoundReleaseAlignTable();
  ~GOSoundReleaseAlignTable();

  bool Load(GOCache &cache);
  bool Save(GOCacheWriter &cache);

  void ComputeTable(
    const GOSoundAudioSection &release,
    unsigned phase_align_max_amplitude,
    unsigned phase_align_max_derivative,
    unsigned int sample_rate,
    unsigned start_position);

  unsigned GetPositionFor(
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS], unsigned nChannels) const;
};

#endif /* GOSOUNDRELEASEALIGNTABLE_H */
