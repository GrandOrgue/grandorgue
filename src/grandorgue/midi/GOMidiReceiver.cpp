/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiver.h"

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"

GOMidiReceiver::GOMidiReceiver(
  GOOrganModel &organModel, GOMidiReceiverType type)
  : GOMidiReceiverBase(type), r_config(organModel.GetConfig()) {}

void GOMidiReceiver::Load(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &map) {
  if (!r_config.ODFCheck()) {
    /* Skip old style entries */
    if (m_type == MIDI_RECV_DRAWSTOP)
      cfg.ReadInteger(
        ODFSetting, group, wxT("StopControlMIDIKeyNumber"), -1, 127, false);
    if (m_type == MIDI_RECV_BUTTON || m_type == MIDI_RECV_SETTER)
      cfg.ReadInteger(
        ODFSetting, group, wxT("MIDIProgramChangeNumber"), 0, 128, false);
  }
  GOMidiReceiverBase::Load(cfg, group, map);
}

int GOMidiReceiver::GetTranspose() const { return r_config.Transpose(); }
