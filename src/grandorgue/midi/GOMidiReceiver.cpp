/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
  : GOMidiReceiverBase(type),
    r_OrganModel(organModel),
    r_config(organModel.GetConfig()),
    m_Index(-1) {}

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

void GOMidiReceiver::Preconfigure(GOConfigReader &cfg, wxString group) {
  unsigned index = 0;

  if (m_type == MIDI_RECV_SETTER) {
    index = m_Index;
  }
  if (m_type == MIDI_RECV_MANUAL) {
    if (m_Index == -1)
      return;

    index = r_OrganModel.GetManual(m_Index)->GetMIDIInputNumber();
  }
  if (m_type == MIDI_RECV_ENCLOSURE) {
    if (m_Index == -1)
      return;

    index = r_OrganModel.GetEnclosureElement(m_Index)->GetMIDIInputNumber();
  }
  const GOMidiReceiverBase *recv = r_config.FindMidiEvent(m_type, index);

  if (!recv)
    return;

  for (unsigned i = 0; i < recv->GetEventCount(); i++)
    m_events.push_back(recv->GetEvent(i));
}

int GOMidiReceiver::GetTranspose() { return r_config.Transpose(); }
