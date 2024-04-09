/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundProviderSynthedTrem.h"

#include "GOBuffer.h"
#include "GOMemoryPool.h"
#include "GOSoundAudioSection.h"

GOSoundProviderSynthedTrem::GOSoundProviderSynthedTrem() { m_Gain = 1.0f; }

inline short SynthTrem(double amp, double angle) {
  return (short)(amp * sin(angle));
}

inline short SynthTrem(double amp, double angle, double fade) {
  if (fade > 1.0)
    fade = 1.0;
  if (fade < 0.0)
    fade = 0.0;
  return (short)(fade * amp * sin(angle));
}

void GOSoundProviderSynthedTrem::Create(
  GOMemoryPool &pool,
  int period,
  int start_rate,
  int stop_rate,
  int amp_mod_depth) {
  ClearData();

  const double trem_freq = 1000.0 / period;
  const int sample_freq = 44100;

  const unsigned attack_samples = sample_freq / start_rate;
  const unsigned loop_samples = sample_freq / trem_freq;
  const unsigned release_samples = sample_freq / stop_rate;
  const unsigned total_samples
    = attack_samples + loop_samples + release_samples;

  /* Synthesize tremulant */
  const double pi = 3.14159265358979323846;
  const double trem_amp = (0x7FF0 * amp_mod_depth / 100);
  const double trem_param = 2 * pi / loop_samples;
  double trem_angle = 0.0;

  GOBuffer<int16_t> data(total_samples);

  int16_t *write_iterator = data.get();

  /* Synthesize attack */
  double trem_fade = 0.0;
  double trem_inc = 1.0 / attack_samples;
  for (unsigned i = 0; i < attack_samples; i++, write_iterator++) {
    *write_iterator = SynthTrem(trem_amp, trem_angle, trem_fade);
    trem_angle += trem_param * trem_fade;
    trem_fade += trem_inc;
  }

  /* Synthesize loop */
  for (unsigned i = 0; i < loop_samples; i++, write_iterator++) {
    *write_iterator = SynthTrem(trem_amp, trem_angle);
    trem_angle += trem_param;
  }

  /* Synthesize release */
  trem_inc = 1.0 / (double)release_samples;
  trem_fade = 1.0 - trem_inc;
  for (unsigned i = 0; i < release_samples; i++, write_iterator++) {
    *write_iterator = SynthTrem(trem_amp, trem_angle, trem_fade);
    trem_angle += trem_param * trem_fade;
    trem_fade -= trem_inc;
  }

  /* Attack sustain section */
  GOWaveLoop trem_loop;
  trem_loop.m_StartPosition = attack_samples;
  trem_loop.m_EndPosition = (attack_samples + loop_samples) - 1;
  std::vector<GOWaveLoop> trem_loops;
  trem_loops.push_back(trem_loop);
  AttackSelector attack_info;
  attack_info.m_WaveTremulantStateFor = BOOL3_DEFAULT;
  attack_info.min_attack_velocity = 0;
  attack_info.max_released_time = -1;
  m_AttackInfo.push_back(attack_info);
  m_Attack.push_back(new GOSoundAudioSection(pool));
  m_Attack[0]->Setup(
    nullptr,
    nullptr,
    data.get(),
    GOWave::SF_SIGNEDSHORT_16,
    1,
    sample_freq,
    trem_loop.m_EndPosition,
    &trem_loops,
    BOOL3_DEFAULT,
    false,
    0,
    0);

  /* Release section */
  ReleaseSelector release_info;
  release_info.m_WaveTremulantStateFor = BOOL3_DEFAULT;
  release_info.max_playback_time = -1;
  m_ReleaseInfo.push_back(release_info);
  m_Release.push_back(new GOSoundAudioSection(pool));
  m_Release[0]->Setup(
    nullptr,
    nullptr,
    data.get() + attack_samples + loop_samples,
    GOWave::SF_SIGNEDSHORT_16,
    1,
    sample_freq,
    release_samples,
    NULL,
    BOOL3_DEFAULT,
    false,
    0,
    0);

  ComputeReleaseAlignmentInfo();
}
