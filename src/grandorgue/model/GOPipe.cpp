/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipe.h"

#include <wx/intl.h>

#include "GOEventHandlerList.h"
#include "GORank.h"

GOPipe::GOPipe(
  GOEventHandlerList *handlerList, GORank *rank, unsigned midi_key_number)
  : m_Velocity(0),
    m_Velocities(1),
    m_Rank(rank),
    m_MidiKeyNumber(midi_key_number) {
  handlerList->RegisterSoundStateHandler(this);
}

GOPipe::~GOPipe() {}

unsigned GOPipe::RegisterReference(GOPipe *pipe) {
  unsigned id = m_Velocities.size();
  m_Velocities.resize(id + 1);
  return id;
}

void GOPipe::PreparePlayback() {
  m_Velocity = 0;
  for (unsigned i = 0; i < m_Velocities.size(); i++)
    m_Velocities[i] = 0;
}

void GOPipe::SetTemperament(const GOTemperament &temperament) {}

void GOPipe::SetVelocity(unsigned velocity, unsigned referenceID) {
  if (m_Velocities[referenceID] <= velocity && velocity <= m_Velocity) {
    m_Velocities[referenceID] = velocity;
    return;
  }
  unsigned last_velocity = m_Velocity;
  if (velocity >= m_Velocity) {
    m_Velocities[referenceID] = velocity;
    m_Velocity = velocity;
  } else {
    m_Velocities[referenceID] = velocity;
    m_Velocity = m_Velocities[0];
    for (unsigned i = 1; i < m_Velocities.size(); i++)
      if (m_Velocity < m_Velocities[i])
        m_Velocity = m_Velocities[i];
  }

  m_Rank->SendKey(m_MidiKeyNumber, velocity);
  VelocityChanged(m_Velocity, last_velocity);
}
