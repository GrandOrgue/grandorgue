/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDONEPOLEFILTER_H_
#define GOSOUNDONEPOLEFILTER_H_

#include <cmath>
#include <cstdint>

#include "sound/buffer/GOSoundBufferMutable.h"

class GOSoundOnePoleFilter {
public:
  enum class Type : uint8_t {
    TYPE_NONE = 0,
    TYPE_LPF,
    TYPE_HPF,
    TYPE_LOW_SHELF,
    TYPE_HIGH_SHELF
  };
  class FilterState {
  public:
    FilterState() { Init(nullptr); }
    void Init(const GOSoundOnePoleFilter *filter);
    bool IsToApply() { return p_filter && p_filter->IsToApply(); }
    inline void ProcessBuffer(GOSoundBufferMutable &outBuffer) {
      float *pData = outBuffer.GetData();
      const unsigned nFrames = outBuffer.GetNFrames();
      float out[2];

      for (unsigned int i = 0; i < nFrames; i++, pData += 2) {
        out[0] = p_filter->m_B0 * pData[0] + m_state[0];
        out[1] = p_filter->m_B0 * pData[1] + m_state[1];
        m_state[0] = p_filter->m_B1 * pData[0] - p_filter->m_A1 * out[0];
        m_state[1] = p_filter->m_B1 * pData[1] - p_filter->m_A1 * out[1];

        pData[0] = out[0];
        pData[1] = out[1];
      }
    }

  private:
    float m_state[2];
    const GOSoundOnePoleFilter *p_filter;
  };

private:
  Type m_type;
  unsigned m_samplerate;

  // Calculated filter coefficients
  double m_B0;
  double m_B1;
  double m_A1;

public:
  GOSoundOnePoleFilter();
  void Init(Type type, double frequency, double gain = 0);
  bool IsToApply() const { return static_cast<bool>(m_type); }
  void SetSamplerate(unsigned samplerate) {
    m_samplerate = samplerate;
    m_type = Type::TYPE_NONE;
  }
};

#endif /* GOSOUNDONEPOLEFILTER_H_ */
