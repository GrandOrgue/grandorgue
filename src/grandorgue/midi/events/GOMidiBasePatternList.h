/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIBASEPATTERNLIST_H
#define GOMIDIBASEPATTERNLIST_H

#include <cstdint>

class GOMidiEventPattern;

class GOMidiBasePatternList {
public:
  virtual unsigned GetEventCount() const = 0;
  virtual const GOMidiEventPattern &GetBasePattern(unsigned index) const = 0;
};

#endif /* GOMIDIBASEPATTERNLIST_H */
