/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTONEBALANCE_H_
#define GOSOUNDTONEBALANCE_H_

#include "GOSoundFilter.h"
#include <cmath>

class GOSoundToneBalance {
public:
  void Init(int slider) {
    if (slider == 0)
      m_filter.SetIsToApply(false);
    else {
      double hz;
      GO_FILTER_TYPE type;

      if (slider < 0) {
        type = GO_FILTER_TYPE::LPF;
        hz = 16000 * pow(20.0 / 16000.0, abs(slider) / 99.0);
      } else {
        type = GO_FILTER_TYPE::HPF;
        hz = 20 * pow(800.0, slider / 99.0);
      }

      m_filter.SetIsToApply(true);
      m_filter.Init(type, hz);
    }
  }
  const GOSoundFilter *GetFilter() const { return &m_filter; }
  void SetFilterSamplerate(unsigned samplerate) {
    m_filter.SetSamplerate(samplerate);
  }

private:
  GOSoundFilter m_filter;
};

#endif /* GOSOUNDTONEBALANCE_H_ */
