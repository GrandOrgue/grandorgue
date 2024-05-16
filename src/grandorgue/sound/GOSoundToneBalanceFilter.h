/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTONEBALANCEFILTER_H_
#define GOSOUNDTONEBALANCEFILTER_H_

#include "GOSoundFilter.h"

class GOSoundToneBalanceFilter {
public:
  void Init(int8_t value);
  const GOSoundFilter *GetFilter() const { return &m_filter; }
  void SetFilterSamplerate(unsigned samplerate) {
    m_filter.SetSamplerate(samplerate);
  }

private:
  GOSoundFilter m_filter;
};

#endif /* GOSOUNDTONEBALANCE_H_ */
