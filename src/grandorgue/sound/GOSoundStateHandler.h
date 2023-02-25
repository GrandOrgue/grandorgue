/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTATEHANDLER_H
#define GOSOUNDSTATEHANDLER_H

class GOSoundStateHandler {
public:
  virtual ~GOSoundStateHandler() {}

  virtual void AbortPlayback() = 0;
  virtual void PreparePlayback() = 0;
  virtual void StartPlayback() = 0;
  virtual void PrepareRecording() = 0;
};

#endif
