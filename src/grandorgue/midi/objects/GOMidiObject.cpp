/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include "midi/elements/GOMidiReceiver.h"
#include "midi/elements/GOMidiSender.h"
#include "midi/elements/GOMidiShortcutReceiver.h"

#include "GOMidiObjectContext.h"

GOMidiObject::GOMidiObject(
  GOMidiMap &midiMap, const wxString &midiTypeCode, const wxString &midiType)
  : r_MidiMap(midiMap),
    r_MidiTypeCode(midiTypeCode),
    r_MidiTypeName(midiType),
    p_MidiSender(nullptr),
    p_MidiReceiver(nullptr),
    p_ShortcutReceiver(nullptr),
    p_DivisionSender(nullptr),
    p_context(nullptr) {}

wxString GOMidiObject::GetContextTitle() const {
  return GOMidiObjectContext::getFullTitle(p_context);
}

bool GOMidiObject::IsMidiConfigured() const {
  return (p_MidiSender && p_MidiSender->IsMidiConfigured())
    || (p_MidiReceiver && p_MidiReceiver->IsMidiConfigured())
    || (p_ShortcutReceiver && p_ShortcutReceiver->IsMidiConfigured())
    || (p_DivisionSender && p_DivisionSender->IsMidiConfigured());
}

void GOMidiObject::InitMidiObject(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  SetGroup(group);
  m_name = name;
  LoadMidiObject(cfg, group, r_MidiMap);
}