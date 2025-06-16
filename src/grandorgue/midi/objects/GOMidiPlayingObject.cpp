/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPlayingObject.h"

#include <wx/intl.h>

#include "model/GOOrganModel.h"

GOMidiPlayingObject::GOMidiPlayingObject(
  GOOrganModel &organModel, ObjectType objectType)
  : GOMidiObject(organModel.GetConfig().GetMidiMap(), objectType),
    r_OrganModel(organModel) {
  r_OrganModel.RegisterSoundStateHandler(this);
  r_OrganModel.RegisterMidiObject(this);
  r_OrganModel.RegisterSaveableObject(this);
}

GOMidiPlayingObject::~GOMidiPlayingObject() {
  r_OrganModel.UnregisterSaveableObject(this);
  r_OrganModel.UnRegisterMidiObject(this);
  r_OrganModel.UnRegisterSoundStateHandler(this);
}

void GOMidiPlayingObject::ShowConfigDialog() {
  const bool isReadOnly = IsReadOnly();
  const wxString title = wxString::Format(
    _("MIDI-Settings for %s - %s"), GetMidiTypeName(), GetName());
  const wxString selector
    = wxString::Format(wxT("%s.%s"), GetMidiTypeCode(), GetName());

  r_OrganModel.ShowMIDIEventDialog(
    this,
    title,
    selector,
    isReadOnly ? nullptr : GetMidiReceiver(),
    GetMidiSender(),
    isReadOnly ? nullptr : GetMidiShortcutReceiver(),
    GetDivisionSender(),
    this);
}