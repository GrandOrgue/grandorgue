/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDICALLBACK_H
#define GOMIDICALLBACK_H

class GOMidiEvent;

class GOMidiCallback {
public:
  virtual ~GOMidiCallback() {}

  virtual void OnMidiEvent(const GOMidiEvent &event) = 0;
};

#endif
