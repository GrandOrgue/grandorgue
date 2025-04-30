/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GORegisteredOrgan.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GORegisteredOrgan::GORegisteredOrgan(const GOOrgan &organ)
  : GOOrgan(organ), m_midi(MIDI_RECV_ORGAN) {
  const GORegisteredOrgan *pRegOrgan
    = dynamic_cast<const GORegisteredOrgan *>(&organ);

  if (pRegOrgan)
    m_midi = pRegOrgan->m_midi;
}

GORegisteredOrgan::GORegisteredOrgan(
  GOConfig &config, GOConfigReader &cfg, const wxString &group)
  : GOOrgan(
    cfg.ReadString(CMBSetting, group, wxT("ODFPath")),
    cfg.ReadString(CMBSetting, group, wxT("Archiv"), false),
    cfg.ReadString(CMBSetting, group, wxT("ArchivePath"), false),
    cfg.ReadString(CMBSetting, group, wxT("ChurchName")),
    cfg.ReadString(CMBSetting, group, wxT("OrganBuilder")),
    cfg.ReadString(CMBSetting, group, wxT("RecordingDetail"))),
    m_midi(config, MIDI_RECV_ORGAN) {
  m_LastUse = cfg.ReadInteger(
    CMBSetting, group, wxT("LastUse"), 0, INT_MAX, false, m_LastUse);
  m_midi.Load(cfg, group, config.GetMidiMap());
}

void GORegisteredOrgan::Save(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const {
  cfg.WriteString(group, wxT("ODFPath"), m_ODF);
  cfg.WriteString(group, wxT("ChurchName"), m_ChurchName);
  cfg.WriteString(group, wxT("OrganBuilder"), m_OrganBuilder);
  cfg.WriteString(group, wxT("RecordingDetail"), m_RecordingDetail);
  if (m_ArchiveID != wxEmptyString) {
    cfg.WriteString(group, wxT("Archiv"), m_ArchiveID);
    cfg.WriteString(group, wxT("ArchivePath"), m_ArchivePath);
  }
  cfg.WriteInteger(group, wxT("LastUse"), m_LastUse);
  m_midi.Save(cfg, group, map);
}

bool GORegisteredOrgan::Match(const GOMidiEvent &e) {
  switch (m_midi.Match(e)) {
  case MIDI_MATCH_CHANGE:
  case MIDI_MATCH_ON:
    return true;

  default:
    return false;
  }
}
