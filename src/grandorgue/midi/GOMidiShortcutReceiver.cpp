/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiShortcutReceiver.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

void GOMidiShortcutReceiver::Load(GOConfigReader &cfg, wxString group) {
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

void GOMidiShortcutReceiver::Save(GOConfigWriter &cfg, wxString group) {
  if (m_type == KEY_RECV_ENCLOSURE) {
    cfg.WriteInteger(group, wxT("PlusKey"), m_ShortcutKey);
    cfg.WriteInteger(group, wxT("MinusKey"), m_MinusKey);
  } else {
    cfg.WriteInteger(group, wxT("ShortcutKey"), m_ShortcutKey);
  }
}

KEY_MATCH_TYPE GOMidiShortcutReceiver::Match(unsigned key) {
  if (m_ShortcutKey == key)
    return KEY_MATCH;
  if (m_MinusKey == key)
    return KEY_MATCH_MINUS;
  return KEY_MATCH_NONE;
}

bool GOMidiShortcutReceiver::RenewFrom(
  const GOMidiShortcutPattern &newPattern) {
  GOMidiShortcutPattern &pattern = *this;
  bool result = !(newPattern == pattern);

  if (result)
    pattern = newPattern;
  return result;
}
