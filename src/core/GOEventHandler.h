/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTHANDLER_H
#define GOEVENTHANDLER_H

class GOMidiEvent;

class GOEventHandler {
public:
  virtual ~GOEventHandler() {}

  virtual void ProcessMidi(const GOMidiEvent &event) = 0;
  virtual void HandleKey(int key) = 0;
};

#endif
