/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROVIDERWAVE_H_
#define GOSOUNDPROVIDERWAVE_H_

#include <wx/string.h>

#include <vector>

#include "loader/GOLoaderFilename.h"

#include "GOSoundProvider.h"
#include "GOWaveLoop.h"

class GOWave;

typedef enum {
  /* Only the first loop with the earliest endpoint is loaded. This will
   * result in the minimal amount of memory being loaded for the sample.
   */
  LOOP_LOAD_CONSERVATIVE = 0,

  /* Only the longest loop will be loaded. This only provides a benefit if
   * the longest loop is not the last loop found.
   */
  LOOP_LOAD_LONGEST = 1,

  /* Stores all samples up to the very last loop end-point. Uses the most
   * memory and should achieve best realism.
   */
  LOOP_LOAD_ALL = 2
} loop_load_type;

typedef struct {
  GOLoaderFilename filename;
  int sample_group;
  bool load_release;
  bool percussive;
  unsigned min_attack_velocity;
  unsigned max_released_time;
  int max_playback_time;
  int attack_start;
  int cue_point;
  int release_end;
  std::vector<GOWaveLoop> loops;
} attack_load_info;

typedef struct {
  GOLoaderFilename filename;
  int sample_group;
  int max_playback_time;
  int cue_point;
  int release_end;
} release_load_info;

class GOSoundProviderWave : public GOSoundProvider {
  unsigned GetBytesPerSample(unsigned bits_per_sample);

  void CreateAttack(
    GOMemoryPool &pool,
    const char *data,
    GOWave &wave,
    int attack_start,
    const std::vector<GOWaveLoop> *pSrcLoops,
    int sample_group,
    unsigned bits_per_sample,
    unsigned channels,
    bool compress,
    loop_load_type loop_mode,
    bool percussive,
    unsigned min_attack_velocity,
    unsigned loop_crossfade_length,
    unsigned max_released_time);

  void CreateRelease(
    GOMemoryPool &pool,
    const char *data,
    GOWave &wave,
    int sample_group,
    unsigned max_playback_time,
    int cue_point,
    int release_end,
    unsigned bits_per_sample,
    unsigned channels,
    bool compress);

  /*
   * Load attack and/or release samples from one wav file or from an archive
   */
  void LoadFromOneFile(
    GOMemoryPool &pool,
    GOOpenedFile *file,
    const std::vector<GOWaveLoop> *loops,
    bool is_attack,
    bool is_release,
    int sample_group,
    unsigned max_playback_time,
    int attack_start,
    int cue_point,
    int release_end,
    unsigned bits_per_sample,
    int load_channels,
    bool compress,
    loop_load_type loop_mode,
    bool percussive,
    unsigned min_attack_velocity,
    bool use_pitch,
    unsigned loop_crossfade_length,
    unsigned max_released_time);

  void LoadPitch(GOOpenedFile *file);
  unsigned GetFaderLength(unsigned MidiKeyNumber);

public:
  /*
   * Load all attack and release samples from corresponding .wav files or from
   * an archive
   */
  void LoadFromMultipleFiles(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    std::vector<attack_load_info> attacks,
    std::vector<release_load_info> releases,
    unsigned bits_per_sample,
    int channels,
    bool compress,
    loop_load_type loop_mode,
    unsigned attack_load,
    unsigned release_load,
    unsigned loop_crossfade_length,
    unsigned release_crossfase_length);
  void SetAmplitude(float fixed_amplitude, float gain);
};

#endif /* GOSOUNDPROVIDERWAVE_H_ */
