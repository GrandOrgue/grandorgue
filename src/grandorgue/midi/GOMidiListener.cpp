/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiListener.h"

#include "GOMidi.h"
#include "midi/GOMidiCallback.h"
#include "midi/GOMidiEvent.h"

GOMidiListener::GOMidiListener() : m_Callback(NULL), m_midi(NULL) {}

GOMidiListener::~GOMidiListener() { Unregister(); }

void GOMidiListener::SetCallback(GOMidiCallback *callback) {
  m_Callback = callback;
}

void GOMidiListener::Register(GOMidi *midi) {
  Unregister();
  if (midi) {
    m_midi = midi;
    m_midi->Register(this);
  }
}

void GOMidiListener::Unregister() {
  if (m_midi)
    m_midi->Unregister(this);
  m_midi = NULL;
}

void GOMidiListener::Send(const GOMidiEvent &event) {
  if (m_Callback)
    m_Callback->OnMidiEvent(event);
}
