/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROVIDERWAVE_H_
#define GOSOUNDPROVIDERWAVE_H_

#include <wx/string.h>

#include <vector>

#include "GOFilename.h"
#include "GOSoundProvider.h"

class GOWave;
typedef struct GO_WAVE_LOOP GO_WAVE_LOOP;

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
  int loop_start;
  int loop_end;
} loop_load_info;

typedef struct {
  GOFilename filename;
  int sample_group;
  bool load_release;
  bool percussive;
  unsigned min_attack_velocity;
  unsigned max_released_time;
  int max_playback_time;
  int attack_start;
  int cue_point;
  int release_end;
  std::vector<loop_load_info> loops;
} attack_load_info;

typedef struct {
  GOFilename filename;
  int sample_group;
  int max_playback_time;
  int cue_point;
  int release_end;
} release_load_info;

class GOSoundProviderWave : public GOSoundProvider {
  unsigned GetBytesPerSample(unsigned bits_per_sample);

  void CreateAttack(
    const char *data,
    GOWave &wave,
    int attack_start,
    std::vector<GO_WAVE_LOOP> loop_list,
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
    const char *data,
    GOWave &wave,
    int sample_group,
    unsigned max_playback_time,
    int cue_point,
    int release_end,
    unsigned bits_per_sample,
    unsigned channels,
    bool compress);
  void ProcessFile(
    const GOFilename &filename,
    std::vector<GO_WAVE_LOOP> loops,
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
  void LoadPitch(const GOFilename &filename);
  unsigned GetFaderLength(unsigned MidiKeyNumber);

 public:
  GOSoundProviderWave(GOMemoryPool &pool);

  void LoadFromFile(
    std::vector<attack_load_info> attacks,
    std::vector<release_load_info> releases,
    unsigned bits_per_sample,
    int channels,
    bool compress,
    loop_load_type loop_mode,
    unsigned attack_load,
    unsigned release_load,
    int midi_key_number,
    unsigned loop_crossfade_length,
    unsigned release_crossfase_length);
  void SetAmplitude(float fixed_amplitude, float gain);
};

#endif /* GOSOUNDPROVIDERWAVE_H_ */
