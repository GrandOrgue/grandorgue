/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReleaseAlignTable.h"

#include <stdlib.h>

#include "loader/cache/GOCache.h"
#include "loader/cache/GOCacheWriter.h"
#include "sound/GOSoundAudioSection.h"

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
#include <stdio.h>
#endif
#endif

GOSoundReleaseAlignTable::GOSoundReleaseAlignTable() {
  memset(m_PositionEntries, 0, sizeof(m_PositionEntries));
  m_PhaseAlignMaxAmplitude = 0;
  m_PhaseAlignMaxDerivative = 0;
}

GOSoundReleaseAlignTable::~GOSoundReleaseAlignTable() {}

bool GOSoundReleaseAlignTable::Load(GOCache &cache) {
  if (!cache.Read(&m_PhaseAlignMaxAmplitude, sizeof(m_PhaseAlignMaxAmplitude)))
    return false;
  if (!cache.Read(
        &m_PhaseAlignMaxDerivative, sizeof(m_PhaseAlignMaxDerivative)))
    return false;
  if (!cache.Read(&m_PositionEntries, sizeof(m_PositionEntries)))
    return false;
  return true;
}

bool GOSoundReleaseAlignTable::Save(GOCacheWriter &cache) {
  if (!cache.Write(&m_PhaseAlignMaxAmplitude, sizeof(m_PhaseAlignMaxAmplitude)))
    return false;
  if (!cache.Write(
        &m_PhaseAlignMaxDerivative, sizeof(m_PhaseAlignMaxDerivative)))
    return false;
  if (!cache.Write(&m_PositionEntries, sizeof(m_PositionEntries)))
    return false;
  return true;
}

void GOSoundReleaseAlignTable::ComputeTable(
  const GOSoundAudioSection &release,
  int phase_align_max_amplitude,
  int phase_align_max_derivative,
  unsigned int sample_rate,
  unsigned start_position) {
  DecompressionCache cache;
  InitDecompressionCache(cache);

  for (unsigned i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
    for (unsigned j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
      m_PositionEntries[i][j] = 0;

  unsigned channels = release.GetChannels();
  m_PhaseAlignMaxDerivative = phase_align_max_derivative;
  m_PhaseAlignMaxAmplitude = phase_align_max_amplitude;

  /* We will use a short portion of the release to analyse to get the
   * release offset table. This length is defined by the
   * PHASE_ALIGN_MIN_FREQUENCY macro and should be set to the lowest
   * frequency pipe you would ever expect... if this length is greater
   * than the length of the release, truncate it */
  unsigned required_search_len = sample_rate / PHASE_ALIGN_MIN_FREQUENCY;
  unsigned release_len = release.GetLength();
  if (release_len < required_search_len + BLOCK_HISTORY + start_position)
    return;
  /* If number of samples in the release is not enough to fill the release
   * table, abort - release alignment probably wont help. */
  if (
    required_search_len < PHASE_ALIGN_AMPLITUDES * PHASE_ALIGN_DERIVATIVES * 2)
    return;

  /* Generate the release table using the small portion of the release... */
  bool found[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
  memset(found, 0, sizeof(found));

  int f_p = 0;
  for (unsigned int j = 0; j < channels; j++)
    f_p += release.GetSample(start_position + (BLOCK_HISTORY - 1), j, &cache);

  for (unsigned i = BLOCK_HISTORY; i < required_search_len; i++) {
    /* Store previous values */
    int f = 0;
    for (unsigned int j = 0; j < channels; j++)
      f += release.GetSample(start_position + i, j, &cache);

    /* Bring v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
    int v_mod = (f - f_p) + m_PhaseAlignMaxDerivative - 1;
    int derivIndex
      = (PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative);

    /* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
    int f_mod = f + m_PhaseAlignMaxAmplitude - 1;
    int ampIndex
      = (PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude);

    /* Store this release point if it was not already found */
    derivIndex = (derivIndex < 0)
      ? 0
      : (
        (derivIndex >= PHASE_ALIGN_DERIVATIVES) ? PHASE_ALIGN_DERIVATIVES - 1
                                                : derivIndex);
    ampIndex = (ampIndex < 0)
      ? 0
      : (
        (ampIndex >= PHASE_ALIGN_AMPLITUDES) ? PHASE_ALIGN_AMPLITUDES - 1
                                             : ampIndex);
    assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
    assert((ampIndex >= 0) && (ampIndex < PHASE_ALIGN_AMPLITUDES));
    if (!found[derivIndex][ampIndex]) {
      m_PositionEntries[derivIndex][ampIndex] = i + 1 + start_position;
      found[derivIndex][ampIndex] = true;
    }

    f_p = f;
  }

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
  /* print some phase debugging information */
  for (unsigned int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++) {
    printf("deridx: %d\n", i);
    for (unsigned int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
      if (found[i][j])
        printf("  idx %d: found\n", j);
      else
        printf("  idx %d: not found\n", j);
  }
#endif
#endif

  /* Phase 2, if there are any entries in the table which were not found,
   * attempt to fill them with the nearest available value. */
  for (int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
    for (int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
      if (!found[i][j]) {
        bool foundsecond = false;
        for (int l = 0; (l < 2 * PHASE_ALIGN_DERIVATIVES) && (!foundsecond);
             l++)
          for (int k = 0; (k < 2 * PHASE_ALIGN_AMPLITUDES) && (!foundsecond);
               k++) {
            foundsecond = true;
            int sl = (l + 1) / 2;
            if ((l & 1) == 0)
              sl = -sl;
            int sk = (k + 2) / 2;
            if ((k & 1) == 0)
              sk = -sk;
            if (
              (i + sl < PHASE_ALIGN_DERIVATIVES) && (i + sl >= 0)
              && (j + sk < PHASE_ALIGN_AMPLITUDES) && (j + sk >= 0)
              && (found[i + sl][j + sk])) {
              m_PositionEntries[i][j] = m_PositionEntries[i + sl][j + sk];
            } else {
              foundsecond = false;
            }
          }

        assert(foundsecond);
        foundsecond = false;
      }
}

void GOSoundReleaseAlignTable::SetupRelease(
  audio_section_stream &release_sampler,
  const audio_section_stream &old_sampler) const {
  int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];
  GOSoundAudioSection::GetHistory(&old_sampler, history);

  /* Get combined release f's and v's */
  int f_mod = 0;
  int v_mod = 0;
  for (unsigned i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
    f_mod += history[(BLOCK_HISTORY - 1)][i];
    v_mod += history[(BLOCK_HISTORY - 2)][i];
  }
  v_mod = f_mod - v_mod;

  /* Bring f and v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
  v_mod += (m_PhaseAlignMaxDerivative - 1);
  f_mod += (m_PhaseAlignMaxAmplitude - 1);

  int derivIndex = m_PhaseAlignMaxDerivative
    ? (PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative)
    : PHASE_ALIGN_DERIVATIVES / 2;

  /* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
  int ampIndex = m_PhaseAlignMaxAmplitude
    ? (PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude)
    : PHASE_ALIGN_AMPLITUDES / 2;

  /* Store this release point if it was not already found */
  assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
  assert((ampIndex >= 0) && (ampIndex < PHASE_ALIGN_AMPLITUDES));
  derivIndex = (derivIndex < 0)
    ? 0
    : (
      (derivIndex >= PHASE_ALIGN_DERIVATIVES) ? PHASE_ALIGN_DERIVATIVES - 1
                                              : derivIndex);
  ampIndex = (ampIndex < 0)
    ? 0
    : (
      (ampIndex >= PHASE_ALIGN_AMPLITUDES) ? PHASE_ALIGN_AMPLITUDES - 1
                                           : ampIndex);
  release_sampler.position_index = m_PositionEntries[derivIndex][ampIndex];

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
  printf("setup release using alignment:\n");
  printf("  pos:    %d\n", release_sampler.position_index);
  printf("  derIdx: %d\n", derivIndex);
  printf("  ampIdx: %d\n", ampIndex);
#endif
#endif
}
