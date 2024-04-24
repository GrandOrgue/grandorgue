/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDEQ_H_
#define GOSOUNDEQ_H_

#include <cmath>

class GOSoundEq {
public:
  class EqState {
  public:
    EqState() { Init(NULL); }
    void Init(const GOSoundEq *eq) {
      m_eq = eq;
      for (int i = 0; i < 2; i++)
        m_state[i] = 0;
    }
    bool IsToApplyEq() { return m_eq->IsToApplyEq(); }
    void ProcessBuffer(unsigned n_blocks, float *buffer);

  private:
    float m_state[2];
    const GOSoundEq *m_eq;
  };

private:
  // Calculated filter coefficients
  double m_B0;
  double m_B1;
  double m_A1;

  bool m_isToApply;

public:
  GOSoundEq() { InitEq(0, 0); }
  void InitEq(int slider, unsigned samplerate);
  bool IsToApplyEq() const { return m_isToApply; }
};

inline void GOSoundEq::InitEq(int slider, unsigned samplerate) {
  m_B0 = 0;
  m_B1 = 0;
  m_A1 = 0;
  m_isToApply = false;

  if (slider == 0 || samplerate == 0) {
    return;
  } else {
    double a0, a1, b0, b1;
    if (slider < 0) {
      // lowpass filter act as a high cut
      double hz = 16000 * pow(20.0 / 16000.0, abs(slider) / 99.0);
      double w0 = 2.0 * 3.14159265358979323846 * hz / samplerate;
      double cosW0 = cos(w0);
      double sinW0 = sin(w0);
      b0 = sinW0;
      b1 = sinW0;
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
    } else {
      // highpass filter act as a low cut
      double hz = 20 * pow(800.0, slider / 99.0);
      double w0 = 2.0 * 3.14159265358979323846 * hz / samplerate;
      double cosW0 = cos(w0);
      double sinW0 = sin(w0);
      b0 = 1.0 + cosW0;
      b1 = -(1.0 + cosW0);
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
    }
    m_B0 = b0 / a0;
    m_B1 = b1 / a0;
    m_A1 = a1 / a0;
    m_isToApply = true;
  }
}

inline void GOSoundEq::EqState::ProcessBuffer(
  unsigned n_blocks, float *buffer) {
  float out[2];
  for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
    out[0] = m_eq->m_B0 * buffer[0] + m_state[0];
    out[1] = m_eq->m_B0 * buffer[1] + m_state[1];
    m_state[0] = m_eq->m_B1 * buffer[0] - m_eq->m_A1 * out[0];
    m_state[1] = m_eq->m_B1 * buffer[1] - m_eq->m_A1 * out[1];

    buffer[0] = out[0];
    buffer[1] = out[1];
  }
}

#endif /* GOSOUNDEQ_H_ */
