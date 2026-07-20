/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCLOSELISTENER_H
#define GOSOUNDCLOSELISTENER_H

/**
 * Interface for objects that need to be notified before the sound system
 * closes its audio ports. Implement this to perform cleanup (e.g. stopping
 * the organ engine) before audio output becomes unavailable.
 */
class GOSoundCloseListener {
public:
  virtual ~GOSoundCloseListener() = default;

  /** Called by GOSoundSystem immediately before closing audio ports. */
  virtual void OnBeforeSoundClose() = 0;
};

#endif
