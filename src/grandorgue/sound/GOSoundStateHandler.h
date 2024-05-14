/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTATEHANDLER_H
#define GOSOUNDSTATEHANDLER_H

class GOSoundEngine;

/**
 * This is a basic class for all objects that need to interact with
 * GOSoundEngine.
 * When the sound engine becomes ready, PreparePlayback() is called. When the
 * sound engine goes to be closed, AbortPlayback() is called.
 * GetSoundEngine() can be used for access to the sound engine.
 */
class GOSoundStateHandler {

private:
  GOSoundEngine *p_SoundEngine = nullptr;

  void SetSoundEngine(GOSoundEngine *pSoundEngine) {
    p_SoundEngine = pSoundEngine;
  }

protected:
  // Derived classes should override
  virtual void PreparePlayback() = 0;
  virtual void StartPlayback() {}
  virtual void AbortPlayback() {}
  virtual void PrepareRecording() {}

public:
  /**
   * @return The GOSoundEngine instance when it is ready for playback, otherwise
   *   nullptr
   */
  GOSoundEngine *GetSoundEngine() const { return p_SoundEngine; }

public:
  virtual ~GOSoundStateHandler() { SetSoundEngine(nullptr); }

  void PreparePlaybackExt(GOSoundEngine *pSoundEngine);
  void StartPlaybackExt() { StartPlayback(); }
  void AbortPlaybackExt();
  void PrepareRecordingExt() { PrepareRecording(); }
};

#endif
