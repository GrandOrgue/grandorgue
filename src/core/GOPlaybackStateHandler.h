/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPLAYBACKSTATEHANDLER_H
#define GOPLAYBACKSTATEHANDLER_H

class GOPlaybackStateHandler {
 public:
  virtual ~GOPlaybackStateHandler() {}

  virtual void AbortPlayback() = 0;
  virtual void PreparePlayback() = 0;
  virtual void StartPlayback() = 0;
  virtual void PrepareRecording() = 0;
};

#endif
