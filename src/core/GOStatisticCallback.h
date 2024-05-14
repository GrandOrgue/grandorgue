/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTATISTICCALLBACK_H
#define GOSTATISTICCALLBACK_H

class GOSampleStatistic;

class GOStatisticCallback {
public:
  virtual ~GOStatisticCallback() {}

  virtual GOSampleStatistic GetStatistic() = 0;
};

#endif
