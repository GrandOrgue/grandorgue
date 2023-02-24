/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTATEHANDLER_H
#define GOSOUNDSTATEHANDLER_H

class GOSoundEngine;

class GOSoundStateHandler {
private:
  GOSoundEngine *p_SoundEngine = nullptr;

  void SetSoundEngine(GOSoundEngine *pSoundEngine) {
    p_SoundEngine = pSoundEngine;
  }

protected:
  virtual void PreparePlayback() = 0;

public:
  GOSoundEngine *GetSoundEngine() const { return p_SoundEngine; }

public:
  virtual ~GOSoundStateHandler() { SetSoundEngine(nullptr); }

  void PreparePlayback(GOSoundEngine *pSoundEngine) {
    SetSoundEngine(pSoundEngine);
    PreparePlayback();
  }
  virtual void AbortPlayback() { SetSoundEngine(nullptr); }
  virtual void PrepareRecording() {}
};

#endif
