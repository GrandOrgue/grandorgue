/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRELEASEALIGNTABLE_H
#define GOSOUNDRELEASEALIGNTABLE_H

#include <optional>

#include "GOSoundAudioSection.h"

class GOCache;
class GOCacheWriter;
class GOTestReleaseAlignTable;

#define PHASE_ALIGN_DERIVATIVES 2
#define PHASE_ALIGN_AMPLITUDES 32
#define PHASE_ALIGN_MIN_FREQUENCY 20 /* Hertz */

class GOTestReleaseAlignTable;

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

  /** Row/column coordinates into the phase-align lookup table. */
  struct CellCoords {
    unsigned derivI;
    unsigned ampI;
  };

  /**
   * Searches outward from @p target for a cell whose entry in
   * @p areCellsFilled is true, and returns its coordinates.
   *
   * Algorithm: separable (row-major) outward scan. The derivative dimension
   * is the outer loop and the amplitude dimension is the inner loop; both
   * iterate offsets in symmetric order 0, +1, -1, +2, -2, .... Consequently
   * the entire row at |Δderiv| = d is exhausted before any row at
   * |Δderiv| = d + 1, giving derivative-proximity priority over
   * amplitude-proximity (not a true Chebyshev-nearest search). For
   * PHASE_ALIGN_DERIVATIVES = 2 this degenerates to "same row first, then
   * the other row".
   *
   * @param areCellsFilled 2D table marking which cells already hold a valid
   *                       value.
   * @param target         Coordinates of the target cell being filled.
   * @return Coordinates of the nearest filled neighbor, or @c std::nullopt
   *         if the @p areCellsFilled table is entirely empty.
   */
  static std::optional<CellCoords> findNearestFilledCell(
    const bool areCellsFilled[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES],
    CellCoords target);

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
