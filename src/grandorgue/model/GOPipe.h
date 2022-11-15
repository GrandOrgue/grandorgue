/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPE_H
#define GOPIPE_H

#include <wx/string.h>

#include <vector>

#include "GOPlaybackStateHandler.h"

class GOConfigReader;
class GOEventHandlerList;
class GORank;
class GOTemperament;

class GOPipe : private GOPlaybackStateHandler {
private:
  unsigned m_Velocity;
  std::vector<unsigned> m_Velocities;

protected:
  GORank *m_Rank;
  unsigned m_MidiKeyNumber;

  virtual void Change(unsigned velocity, unsigned old_velocity) = 0;

  void AbortPlayback();
  void StartPlayback();
  void PreparePlayback();
  void PrepareRecording();

public:
  GOPipe(
    GOEventHandlerList *handlerList, GORank *rank, unsigned midi_key_number);
  virtual ~GOPipe();
  virtual void Load(GOConfigReader &cfg, wxString group, wxString prefix) = 0;
  void Set(unsigned velocity, unsigned referenceID = 0);
  unsigned RegisterReference(GOPipe *pipe);
  virtual void SetTemperament(const GOTemperament &temperament);
};

#endif
