/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPEUPDATECALLBACK_H
#define GOPIPEUPDATECALLBACK_H

class GOPipeUpdateCallback {
public:
  virtual ~GOPipeUpdateCallback() {}

  virtual void UpdateAmplitude() = 0;
  virtual void UpdateTuning() = 0;
  virtual void UpdateAudioGroup() = 0;
  virtual void UpdateReleaseTail() = 0;
};

#endif
