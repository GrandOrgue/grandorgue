/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOKeyReceiver.h"

#include "GOOrganController.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOKeyReceiver::GOKeyReceiver(
  GODefinitionFile *organfile, KEY_RECEIVER_TYPE type)
  : GOKeyReceiverData(type), m_organfile(organfile) {}

void GOKeyReceiver::Load(GOConfigReader &cfg, wxString group) {
  if (m_type == KEY_RECV_ENCLOSURE) {
    m_ShortcutKey
      = cfg.ReadInteger(CMBSetting, group, wxT("PlusKey"), 0, 255, false, 0);
    m_MinusKey
      = cfg.ReadInteger(CMBSetting, group, wxT("MinusKey"), 0, 255, false, 0);
  } else {
    m_ShortcutKey = cfg.ReadInteger(
      ODFSetting, group, wxT("ShortcutKey"), 0, 255, false, m_ShortcutKey);
    m_ShortcutKey = cfg.ReadInteger(
      CMBSetting, group, wxT("ShortcutKey"), 0, 255, false, m_ShortcutKey);
  }
}

void GOKeyReceiver::Save(GOConfigWriter &cfg, wxString group) {
  if (m_type == KEY_RECV_ENCLOSURE) {
    cfg.WriteInteger(group, wxT("PlusKey"), m_ShortcutKey);
    cfg.WriteInteger(group, wxT("MinusKey"), m_MinusKey);
  } else {
    cfg.WriteInteger(group, wxT("ShortcutKey"), m_ShortcutKey);
  }
}

KEY_MATCH_TYPE GOKeyReceiver::Match(unsigned key) {
  if (m_ShortcutKey == key)
    return KEY_MATCH;
  if (m_MinusKey == key)
    return KEY_MATCH_MINUS;
  return KEY_MATCH_NONE;
}

void GOKeyReceiver::Assign(const GOKeyReceiverData &data) {
  *(GOKeyReceiverData *)this = data;
  if (m_organfile)
    m_organfile->Modified();
}
