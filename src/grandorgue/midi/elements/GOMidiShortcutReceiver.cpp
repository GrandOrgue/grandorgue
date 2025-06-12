/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiShortcutReceiver.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "yaml/go-wx-yaml.h"

#include "GOKeyConvert.h"

const GOConfigEnum GOMidiShortcutReceiver::SHORTCUT_RECEIVER_TYPES({
  {wxT("Button"), KEY_RECV_BUTTON},
  {wxT("Enclosure"), KEY_RECV_ENCLOSURE},
});

void GOMidiShortcutReceiver::Load(GOConfigReader &cfg, const wxString &group) {
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

void GOMidiShortcutReceiver::Save(
  GOConfigWriter &cfg, const wxString &group) const {
  if (m_ShortcutKey) {
    if (m_type == KEY_RECV_ENCLOSURE) {
      cfg.WriteInteger(group, wxT("PlusKey"), m_ShortcutKey);
      if (m_MinusKey)
        cfg.WriteInteger(group, wxT("MinusKey"), m_MinusKey);
    } else {
      cfg.WriteInteger(group, wxT("ShortcutKey"), m_ShortcutKey);
    }
  }
}

static const char *C_PLUS_KEY = "plus_key";
static const char *C_MINUS_KEY = "minus_key";
static const char *C_SHORTCUT_KEY = "shortcut_key";

void GOMidiShortcutReceiver::ToYaml(
  YAML::Node &yamlNode, GOMidiMap &map) const {
  if (m_ShortcutKey) {
    if (m_type == KEY_RECV_ENCLOSURE) {
      yamlNode[C_PLUS_KEY] = GOKeyConvert::SHORTCUTS.GetName(m_ShortcutKey);
      if (m_MinusKey)
        yamlNode[C_MINUS_KEY] = GOKeyConvert::SHORTCUTS.GetName(m_MinusKey);
    } else
      yamlNode[C_SHORTCUT_KEY] = GOKeyConvert::SHORTCUTS.GetName(m_ShortcutKey);
  }
}

void GOMidiShortcutReceiver::FromYaml(
  const YAML::Node &yamlNode,
  const wxString &path,
  GOMidiMap &map,
  GOStringSet &usedPaths) {
  m_MinusKey = 0;
  m_ShortcutKey = 0;
  if (yamlNode.IsDefined() && yamlNode.IsMap()) {
    if (m_type == KEY_RECV_ENCLOSURE) {
      m_ShortcutKey = (unsigned)read_enum(
        yamlNode,
        path,
        C_PLUS_KEY,
        GOKeyConvert::SHORTCUTS,
        false,
        0,
        usedPaths);
      m_MinusKey = (unsigned)read_enum(
        yamlNode,
        path,
        C_MINUS_KEY,
        GOKeyConvert::SHORTCUTS,
        false,
        0,
        usedPaths);
    } else
      m_ShortcutKey = (unsigned)read_enum(
        yamlNode,
        path,
        C_SHORTCUT_KEY,
        GOKeyConvert::SHORTCUTS,
        false,
        0,
        usedPaths);
  }
}

GOMidiShortcutReceiver::MatchType GOMidiShortcutReceiver::Match(unsigned key) {
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
