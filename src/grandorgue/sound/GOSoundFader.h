/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

class GOSoundFader {
private:
  float m_gain;
  float m_attack;
  float m_decay;
  float m_target;
  float m_VelocityVolume;
  float m_real_target;
  float m_last_volume;
  unsigned m_nb_attack_frames_left;

  void NewAttacking(float target_gain, unsigned n_frames);

  inline void StartDecay(unsigned n_frames) {
    m_decay = -(m_target / n_frames);
  }

public:
  inline void NewAttacking(
    float target_gain, unsigned ms, unsigned sample_rate) {
    NewAttacking(target_gain, sample_rate * ms / 1000);
  }

  void NewConstant(float gain);

  inline void StartDecay(unsigned ms, unsigned sample_rate) {
    StartDecay(sample_rate * ms / 1000);
  }

  inline void SetVelocityVolume(float volume) { m_VelocityVolume = volume; }

  void Process(unsigned n_blocks, float *buffer, float volume);

  bool IsSilent() const { return (m_gain <= 0.0f); }
};

#endif /* GOSOUNDFADER_H_ */
