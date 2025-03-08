/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include <wx/intl.h>

#include "midi/GOMidiReceiver.h"
#include "model/GOOrganModel.h"

GOMidiObject::GOMidiObject(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiType)
  : r_OrganModel(organModel),
    r_MidiMap(organModel.GetConfig().GetMidiMap()),
    r_MidiTypeCode(midiTypeCode),
    r_MidiTypeName(midiType),
    p_MidiSender(nullptr),
    p_MidiReceiver(nullptr),
    p_ShortcutReceiver(nullptr),
    p_DivisionSender(nullptr) {
  r_OrganModel.RegisterSoundStateHandler(this);
  r_OrganModel.RegisterMidiObject(this);
}

GOMidiObject::~GOMidiObject() {
  r_OrganModel.UnregisterSaveableObject(this);
  r_OrganModel.UnRegisterMidiObject(this);
  r_OrganModel.UnRegisterSoundStateHandler(this);
}

void GOMidiObject::InitMidiObject(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  SetGroup(group);
  m_name = name;
  r_OrganModel.RegisterSaveableObject(this);
  LoadMidiObject(cfg, group, r_MidiMap);
}

void GOMidiObject::ShowConfigDialog() {
  const bool isReadOnly = IsReadOnly();
  const wxString title
    = wxString::Format(_("Midi-Settings for %s - %s"), r_MidiTypeName, m_name);
  const wxString selector
    = wxString::Format(wxT("%s.%s"), r_MidiTypeCode, m_name);

  r_OrganModel.ShowMIDIEventDialog(
    this,
    title,
    selector,
    isReadOnly ? nullptr : p_MidiReceiver,
    p_MidiSender,
    isReadOnly ? nullptr : p_ShortcutReceiver,
    p_DivisionSender,
    this);
}
