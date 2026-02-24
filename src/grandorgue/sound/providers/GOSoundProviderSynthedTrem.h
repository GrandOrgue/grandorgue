/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROVIDERSYNTHEDTREM_H_
#define GOSOUNDPROVIDERSYNTHEDTREM_H_

#include "GOSoundProvider.h"

class GOSoundProviderSynthedTrem : public GOSoundProvider {
public:
  GOSoundProviderSynthedTrem();

  void Create(
    GOMemoryPool &pool,
    int period,
    int start_rate,
    int stop_rate,
    int amp_mod_depth);
};

#endif /* GOSOUNDPROVIDERSYNTHEDTREM_H_ */
