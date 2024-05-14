/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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

class GOCacheObject;
class GOWave;

class GOSoundProviderWave : public GOSoundProvider {
public:
  enum LoopLoadType {
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
  };

  struct AttackFileInfo {
    GOLoaderFilename filename;
    std::vector<GOWaveLoop> loops;
    unsigned min_attack_velocity;
    unsigned max_released_time;
    int max_playback_time;
    int attack_start;
    int cue_point;
    int release_end;
    unsigned m_LoopCrossfadeLength;
    unsigned m_ReleaseCrossfadeLength;
    GOBool3 m_WaveTremulantStateFor;
    bool load_release;
    bool percussive;
  };

  struct ReleaseFileInfo {
    GOLoaderFilename filename;
    int max_playback_time;
    int cue_point;
    int release_end;
    unsigned m_ReleaseCrossfadeLength;
    GOBool3 m_WaveTremulantStateFor;
  };

private:
  // Used for error messages
  GOCacheObject *p_ObjectFor;
  unsigned GetBytesPerSample(unsigned bits_per_sample);

  void AddAttackSection(
    GOMemoryPool &pool,
    const GOLoaderFilename &loaderFilename,
    const char *data,
    GOWave &wave,
    int attack_start,
    const std::vector<GOWaveLoop> *pSrcLoops,
    GOBool3 waveTremulantStateFor,
    unsigned bits_per_sample,
    unsigned channels,
    bool compress,
    LoopLoadType loop_mode,
    bool percussive,
    unsigned min_attack_velocity,
    unsigned loop_crossfade_length,
    unsigned max_released_time);

  void AddReleaseSection(
    GOMemoryPool &pool,
    const GOLoaderFilename &loaderFilename,
    const char *data,
    GOWave &wave,
    GOBool3 waveTremulantStateFor,
    unsigned max_playback_time,
    int cue_point,
    int release_end,
    unsigned bits_per_sample,
    unsigned channels,
    bool compress,
    unsigned releaseCrossfadeLength);

  /*
   * Load attack and/or release samples from one wav file or from an archive
   */
  void LoadFromOneFile(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    const GOLoaderFilename &loaderFilename,
    const std::vector<GOWaveLoop> *loops,
    bool is_attack,
    bool is_release,
    GOBool3 waveTremulantStateFor,
    unsigned max_playback_time,
    int attack_start,
    int cue_point,
    int release_end,
    unsigned bits_per_sample,
    int load_channels,
    bool compress,
    LoopLoadType loop_mode,
    bool percussive,
    unsigned min_attack_velocity,
    bool use_pitch,
    unsigned loop_crossfade_length,
    unsigned releaseCrossfadeLengh,
    unsigned max_released_time);

  void LoadPitch(GOOpenedFile *file);

public:
  GOSoundProviderWave(GOCacheObject *pObjectFor = nullptr)
    : p_ObjectFor(pObjectFor) {}

  /*
   * Load all attack and release samples from corresponding .wav files or from
   * an archive
   */
  void LoadFromMultipleFiles(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    std::vector<AttackFileInfo> attacks,
    std::vector<ReleaseFileInfo> releases,
    unsigned bits_per_sample,
    int channels,
    bool compress,
    LoopLoadType loop_mode,
    bool isToLoadAttacks,
    bool isToLoadReleases);
  void SetAmplitude(float fixed_amplitude, float gain);
};

#endif /* GOSOUNDPROVIDERWAVE_H_ */
