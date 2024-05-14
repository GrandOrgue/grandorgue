/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPEWINDCHESTCALLBACK_H
#define GOPIPEWINDCHESTCALLBACK_H

class GOPipeWindchestCallback {
public:
  virtual ~GOPipeWindchestCallback() {}

  virtual void SetWaveTremulant(bool on) = 0;
};

#endif
