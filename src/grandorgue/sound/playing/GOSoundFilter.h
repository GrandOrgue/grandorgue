/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDFILTER_H_
#define GOSOUNDFILTER_H_

#include <cmath>
#include <cstdint>

class GOSoundFilter {
public:
  enum class FilterType : uint8_t {
    TYPE_NONE = 0,
    TYPE_LPF,
    TYPE_HPF,
    TYPE_LOW_SHELF,
    TYPE_HIGH_SHELF
  };
  class FilterState {
  public:
    FilterState() { Init(nullptr); }
    void Init(const GOSoundFilter *filter);
    bool IsToApply() { return p_filter && p_filter->IsToApply(); }
    inline void ProcessBuffer(unsigned n_blocks, float *buffer) {
      float out[2];
      for (unsigned int i = 0; i < n_blocks; i++, buffer += 2) {
        out[0] = p_filter->m_B0 * buffer[0] + m_state[0];
        out[1] = p_filter->m_B0 * buffer[1] + m_state[1];
        m_state[0] = p_filter->m_B1 * buffer[0] - p_filter->m_A1 * out[0];
        m_state[1] = p_filter->m_B1 * buffer[1] - p_filter->m_A1 * out[1];

        buffer[0] = out[0];
        buffer[1] = out[1];
      }
    }

  private:
    float m_state[2];
    const GOSoundFilter *p_filter;
  };

private:
  FilterType m_type;
  unsigned m_samplerate;

  // Calculated filter coefficients
  double m_B0;
  double m_B1;
  double m_A1;

public:
  GOSoundFilter();
  void Init(FilterType type, double frequency, double gain = 0);
  bool IsToApply() const { return static_cast<bool>(m_type); }
  void SetSamplerate(unsigned samplerate) {
    m_samplerate = samplerate;
    m_type = FilterType::TYPE_NONE;
  }
};

#endif /* GOSOUNDFILTER_H_ */
