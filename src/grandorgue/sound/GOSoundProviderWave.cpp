/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundProviderWave.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "files/GOOpenedFile.h"

#include "GOBuffer.h"
#include "GOMemoryPool.h"
#include "GOSoundAudioSection.h"
#include "GOWave.h"

void GOSoundProviderWave::SetAmplitude(float fixed_amplitude, float gain) {
  /* Amplitude is the combination of global amplitude volume and the stop
   * volume. 1000000 would correspond to sample playback at normal volume.
   */
  m_Gain = fixed_amplitude * powf(10.0f, gain * 0.05f);
}

unsigned GOSoundProviderWave::GetBytesPerSample(unsigned bits_per_sample) {
  if (bits_per_sample <= 8)
    return 1;
  else if (bits_per_sample <= 16)
    return 2;
  else
    return 3;
}

void GOSoundProviderWave::CreateAttack(
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
  unsigned max_released_time) {
  std::vector<GOWaveLoop> waveLoops;
  std::vector<GOWaveLoop> loops;
  unsigned attack_pos = attack_start;

  if (!pSrcLoops || pSrcLoops->size() == 0) {
    // loops have not been provided. Read them from wave

    for (unsigned i = 0; i < wave.GetNbLoops(); i++)
      waveLoops.push_back(wave.GetLoop(i));
    pSrcLoops = &waveLoops;
  }

  for (unsigned i = 0; i < pSrcLoops->size(); i++) {
    if (
      (*pSrcLoops)[i].m_StartPosition >= wave.GetLength()
      || (*pSrcLoops)[i].m_StartPosition >= (*pSrcLoops)[i].m_EndPosition
      || (*pSrcLoops)[i].m_EndPosition >= wave.GetLength())
      throw(wxString) _("Invalid loop definition");
    if (
      loop_crossfade_length
      && (*pSrcLoops)[i].m_StartPosition + REMAINING_AFTER_CROSSFADE
          + loop_crossfade_length
        >= (*pSrcLoops)[i].m_EndPosition)
      throw(wxString) _("Loop too short for a cross fade");
  }

  if ((pSrcLoops->size() > 0) && !percussive) {
    switch (loop_mode) {
    case LOOP_LOAD_ALL:
      for (unsigned i = 0; i < pSrcLoops->size(); i++)
        if (attack_pos <= (*pSrcLoops)[i].m_StartPosition)
          loops.push_back((*pSrcLoops)[i]);
      break;
    case LOOP_LOAD_CONSERVATIVE: {
      unsigned cidx = 0;
      for (unsigned i = 1; i < pSrcLoops->size(); i++)
        if ((*pSrcLoops)[i].m_EndPosition < (*pSrcLoops)[i].m_EndPosition)
          if (attack_pos <= (*pSrcLoops)[i].m_StartPosition)
            cidx = i;
      loops.push_back((*pSrcLoops)[cidx]);
    } break;
    default: {
      assert(loop_mode == LOOP_LOAD_LONGEST);

      unsigned lidx = 0;
      for (unsigned int i = 1; i < pSrcLoops->size(); i++) {
        assert((*pSrcLoops)[i].m_EndPosition > (*pSrcLoops)[i].m_StartPosition);
        if (
          ((*pSrcLoops)[i].m_EndPosition - (*pSrcLoops)[i].m_StartPosition)
          > ((*pSrcLoops)[lidx].m_EndPosition - (*pSrcLoops)[lidx].m_StartPosition))
          if (attack_pos <= (*pSrcLoops)[i].m_StartPosition)
            lidx = i;
      }
      loops.push_back((*pSrcLoops)[lidx]);
    }
    }
    if (loops.size() == 0)
      throw(wxString) _("No loop found");
    for (unsigned i = 0; i < loops.size(); i++) {
      loops[i].m_StartPosition -= attack_pos;
      loops[i].m_EndPosition -= attack_pos;
    }
  }

  attack_section_info attack_info;
  attack_info.sample_group = sample_group;
  attack_info.min_attack_velocity = min_attack_velocity;
  attack_info.max_released_time = max_released_time;
  m_AttackInfo.push_back(attack_info);
  GOAudioSection *section = new GOAudioSection(pool);
  m_Attack.push_back(section);
  section->Setup(
    data + attack_pos * GetBytesPerSample(bits_per_sample) * channels,
    (GOWave::SAMPLE_FORMAT)bits_per_sample,
    channels,
    wave.GetSampleRate(),
    wave.GetLength(),
    &loops,
    compress,
    loop_crossfade_length);
}

void GOSoundProviderWave::CreateRelease(
  GOMemoryPool &pool,
  const char *data,
  GOWave &wave,
  int sample_group,
  unsigned max_playback_time,
  int cue_point,
  int release_end,
  unsigned bits_per_sample,
  unsigned channels,
  bool compress) {
  unsigned release_offset
    = wave.HasReleaseMarker() ? wave.GetReleaseMarkerPosition() : 0;
  if (cue_point != -1)
    release_offset = cue_point;
  unsigned release_end_marker = wave.GetLength();
  if (release_end != -1)
    release_end_marker = release_end;
  if (release_end_marker > wave.GetLength())
    throw(wxString) _("Invalid release end position");

  unsigned release_samples = release_end_marker - release_offset;

  if (release_offset >= release_end_marker)
    throw(wxString) _("Invalid release position");

  release_section_info release_info;
  release_info.sample_group = sample_group;
  release_info.max_playback_time = max_playback_time;
  m_ReleaseInfo.push_back(release_info);
  GOAudioSection *section = new GOAudioSection(pool);
  m_Release.push_back(section);
  section->Setup(
    data + release_offset * GetBytesPerSample(bits_per_sample) * channels,
    (GOWave::SAMPLE_FORMAT)bits_per_sample,
    channels,
    wave.GetSampleRate(),
    release_samples,
    NULL,
    compress,
    0);
}

void GOSoundProviderWave::LoadPitch(GOOpenedFile *file) {
  GOWave wave;

  wave.Open(file);
  m_MidiKeyNumber = wave.GetMidiNote();
  m_MidiPitchFract = wave.GetPitchFract();
}

void GOSoundProviderWave::LoadFromOneFile(
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
  unsigned max_released_time) {
  GOWave wave;

  wave.Open(file);

  /* allocate data to work with */
  unsigned totalDataSize = wave.GetLength() * GetBytesPerSample(bits_per_sample)
    * wave.GetChannels();
  GOBuffer<char> data(totalDataSize);

  if (use_pitch) {
    m_MidiKeyNumber = wave.GetMidiNote();
    m_MidiPitchFract = wave.GetPitchFract();
  }

  unsigned channels = wave.GetChannels();
  if (load_channels == 1)
    channels = 1;
  unsigned wave_channels = channels;
  if (load_channels < 0 && (unsigned)-load_channels <= wave.GetChannels()) {
    wave_channels = load_channels;
    channels = 1;
  }
  if (bits_per_sample > wave.GetBitsPerSample())
    bits_per_sample = wave.GetBitsPerSample();

  wave.ReadSamples(
    data.get(),
    (GOWave::SAMPLE_FORMAT)bits_per_sample,
    wave.GetSampleRate(),
    wave_channels);

  if (is_attack)
    CreateAttack(
      pool,
      data.get(),
      wave,
      attack_start,
      loops,
      sample_group,
      bits_per_sample,
      channels,
      compress,
      loop_mode,
      percussive,
      min_attack_velocity,
      loop_crossfade_length,
      max_released_time);

  if (
    is_release
    && (!is_attack || (wave.GetNbLoops() > 0 && wave.HasReleaseMarker() && !percussive)))
    CreateRelease(
      pool,
      data.get(),
      wave,
      sample_group,
      max_playback_time,
      cue_point,
      release_end,
      bits_per_sample,
      channels,
      compress);
}

unsigned GOSoundProviderWave::GetFaderLength(unsigned MidiKeyNumber) {
  unsigned fade_length = 46;
  if (MidiKeyNumber > 0 && MidiKeyNumber < 133) {
    fade_length
      = 184 - (int)((((float)MidiKeyNumber - 42.0f) / 44.0f) * 178.0f);
    if (MidiKeyNumber < 42)
      fade_length = 184;
    if (MidiKeyNumber > 86)
      fade_length = 6;
  }
  return fade_length;
}

void GOSoundProviderWave::LoadFromMultipleFiles(
  const GOFileStore &fileStore,
  GOMemoryPool &pool,
  std::vector<attack_load_info> attacks,
  std::vector<release_load_info> releases,
  unsigned bits_per_sample,
  int load_channels,
  bool compress,
  loop_load_type loop_mode,
  unsigned attack_load,
  unsigned release_load,
  unsigned loop_crossfade_length,
  unsigned release_crossfase_length) {
  ClearData();
  if (!load_channels)
    return;

  bool load_first_attack = true;

  if (!release_load)
    for (int k = -1; k < 2; k++) {
      unsigned longest = 0;
      for (unsigned i = 0; i < attacks.size(); i++)
        if (
          attacks[i].load_release
          && (unsigned)attacks[i].max_playback_time > longest
          && attacks[i].sample_group == k)
          longest = attacks[i].max_playback_time;
      for (unsigned i = 0; i < releases.size(); i++)
        if (
          (unsigned)releases[i].max_playback_time > longest
          && releases[i].sample_group == k)
          longest = releases[i].max_playback_time;

      bool found = false;
      for (unsigned i = 0; i < attacks.size(); i++)
        if (attacks[i].load_release && attacks[i].sample_group == k) {
          if ((unsigned)attacks[i].max_playback_time == longest && !found) {
            found = true;
            continue;
          } else
            attacks[i].load_release = false;
        }
      for (unsigned i = 0; i < releases.size(); i++)
        if (releases[i].sample_group == k) {
          if ((unsigned)releases[i].max_playback_time == longest && !found) {
            found = true;
            continue;
          } else {
            releases[i] = releases[releases.size() - 1];
            releases.resize(releases.size() - 1);
            i--;
          }
        }
    }

  if (!attack_load)
    for (int k = -1; k < 2; k++) {
      int best_idx = -1;
      unsigned min_velocity = 0xff;
      unsigned max_released_time = 0;
      for (unsigned i = 0; i < attacks.size(); i++) {
        if (attacks[i].sample_group != k)
          continue;
        if (attacks[i].min_attack_velocity < min_velocity)
          min_velocity = attacks[i].min_attack_velocity;
        if (attacks[i].max_released_time > max_released_time)
          max_released_time = attacks[i].max_released_time;
        if (best_idx == -1) {
          best_idx = i;
          continue;
        }
        if (attacks[i].load_release && !attacks[best_idx].load_release)
          best_idx = i;
      }
      for (unsigned i = 0; i < attacks.size(); i++) {
        if (attacks[i].sample_group == k) {
          attacks[i].min_attack_velocity = min_velocity;
          attacks[i].max_released_time = max_released_time;
        }
        if (
          attacks[i].sample_group != k || best_idx == -1 || best_idx == (int)i)
          continue;
        if (load_first_attack && i == 0) {
          LoadPitch(attacks[i].filename.Open(fileStore).get());
          load_first_attack = false;
        }
        for (unsigned j = i + 1; j < attacks.size(); j++)
          attacks[j - 1] = attacks[j];
        if ((int)i < best_idx)
          best_idx--;
        attacks.resize(attacks.size() - 1);
        i--;
      }
    }

  try {
    for (unsigned i = 0; i < attacks.size(); i++) {
      LoadFromOneFile(
        pool,
        attacks[i].filename.Open(fileStore).get(),
        &attacks[i].loops,
        true,
        attacks[i].load_release,
        attacks[i].sample_group,
        attacks[i].max_playback_time,
        attacks[i].attack_start,
        attacks[i].cue_point,
        attacks[i].release_end,
        bits_per_sample,
        load_channels,
        compress,
        loop_mode,
        attacks[i].percussive,
        attacks[i].min_attack_velocity,
        load_first_attack,
        loop_crossfade_length,
        attacks[i].max_released_time);
      load_first_attack = false;
    }

    for (unsigned i = 0; i < releases.size(); i++) {
      LoadFromOneFile(
        pool,
        releases[i].filename.Open(fileStore).get(),
        nullptr,
        false,
        true,
        releases[i].sample_group,
        releases[i].max_playback_time,
        0,
        releases[i].cue_point,
        releases[i].release_end,
        bits_per_sample,
        load_channels,
        compress,
        loop_mode,
        true,
        0,
        false,
        loop_crossfade_length,
        0);
    }

    ComputeReleaseAlignmentInfo();
    if (release_crossfase_length)
      m_ReleaseCrossfadeLength = release_crossfase_length;
    else
      m_ReleaseCrossfadeLength = GetFaderLength(m_MidiKeyNumber);
  } catch (...) {
    ClearData();
    throw;
  }
}
