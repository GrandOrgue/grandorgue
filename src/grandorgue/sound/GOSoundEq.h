/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDEQ_H
#define GOSOUNDEQ_H

#include <cmath>

class GOSoundEq {

private:
  // Calculated filter coefficients
  double m_B0;
  double m_B1;
  double m_B2;
  double m_A1;
  double m_A2;

  float m_state1[2];
  float m_state2[2];

  bool m_isToApply;

public:
  void InitEq(int slider, unsigned samplerate);
  void ProcessBuffer(unsigned n_blocks, float *buffer);
  bool IsToApplyEq() { return m_isToApply; }
};

inline void GOSoundEq::InitEq(int slider, unsigned samplerate) {
  m_B0 = 0;
  m_B1 = 0;
  m_B2 = 0;
  m_A1 = 0;
  m_A2 = 0;
  for (int i = 0; i < 2; i++) {
    m_state1[i] = 0;
    m_state2[i] = 0;
  }
  m_isToApply = false;

  if (slider == 0) {
    return;
  } else {
    double a0, a1, a2, b0, b1, b2;
    if (slider < 0) {
      // lowpass filter act as a high cut
      double hz = 4100 + slider * 40;
      double w0 = 2.0 * 3.14159265358979323846 * hz / samplerate;
      double cosW0 = cos(w0);
      double sinW0 = sin(w0);
      double alpha = sinW0 / (2 * (1.0 / sqrt(2.0)));
      b0 = (1.0 - cosW0) / 2.0;
      b1 = 1.0 - cosW0;
      b2 = (1.0 - cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
    } else {
      // highpass filter act as a low cut
      double hz = 20 + slider * 20;
      double w0 = 2.0 * 3.14159265358979323846 * hz / samplerate;
      double cosW0 = cos(w0);
      double sinW0 = sin(w0);
      double alpha = sinW0 / (2 * (1.0 / sqrt(2.0)));
      b0 = (1.0 + cosW0) / 2.0;
      b1 = -(1.0 + cosW0);
      b2 = (1.0 + cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
    }
    m_B0 = b0 / a0;
    m_B1 = b1 / a0;
    m_B2 = b2 / a0;
    m_A1 = a1 / a0;
    m_A2 = a2 / a0;
    m_isToApply = true;
  }
}

inline void GOSoundEq::ProcessBuffer(unsigned n_blocks, float *buffer) {
  float out[2];
  for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
    out[0] = m_B0 * buffer[0] + m_state1[0];
    out[1] = m_B0 * buffer[1] + m_state1[1];
    m_state1[0] = m_B1 * buffer[0] - m_A1 * out[0] + m_state2[0];
    m_state1[1] = m_B1 * buffer[1] - m_A1 * out[1] + m_state2[1];
    m_state2[0] = m_B2 * buffer[0] - m_A2 * out[0];
    m_state2[1] = m_B2 * buffer[1] - m_A2 * out[1];

    buffer[0] = out[0];
    buffer[1] = out[1];
  }
}

#endif /* GOSOUNDEQ_H */
