/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPE_H
#define GOPIPE_H

#include <vector>

#include <wx/string.h>

#include "sound/GOSoundStateHandler.h"

class GOConfigReader;
class GOEventHandlerList;
class GORank;
class GOTemperament;

class GOPipe : protected GOSoundStateHandler {
private:
  unsigned m_Velocity;
  std::vector<unsigned> m_Velocities;

protected:
  GORank *m_Rank;
  unsigned m_MidiKeyNumber;

  /**
   * Called when a user presses or releases the key.
   * @param velocity - the new velocity. >0 - pressed, 0 - released
   * @param old_velocity - the old velocity. If 0, then the kew wasn't pressed
   */
  virtual void VelocityChanged(unsigned velocity, unsigned old_velocity) = 0;

  void PreparePlayback();

public:
  GOPipe(
    GOEventHandlerList *handlerList, GORank *rank, unsigned midi_key_number);
  virtual ~GOPipe();
  virtual void Load(
    GOConfigReader &cfg, const wxString &group, const wxString &prefix)
    = 0;
  /**
   * Called from GORank when a user presses the key
   * @param velocity the velocity value of the midi event
   * @param referenceID - 0 or the id of the reference for a REF: pipe
   */
  void SetVelocity(unsigned velocity, unsigned referenceID = 0);
  unsigned RegisterReference(GOPipe *pipe);
  virtual void SetTemperament(const GOTemperament &temperament);
};

#endif
