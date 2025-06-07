/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISHORTCUTPATTERN_H
#define GOMIDISHORTCUTPATTERN_H

#include "GOMidiShortcutReceiverType.h"

class GOMidiShortcutPattern {
public:
  enum MatchType {
    KEY_MATCH_NONE,
    KEY_MATCH,
    KEY_MATCH_MINUS,
  };

protected:
  GOMidiShortcutReceiverType m_type;
  unsigned m_ShortcutKey = 0;
  unsigned m_MinusKey = 0;

public:
  GOMidiShortcutPattern(GOMidiShortcutReceiverType type) : m_type(type) {}
  virtual ~GOMidiShortcutPattern() {}

  GOMidiShortcutReceiverType GetType() const { return m_type; }

  unsigned GetShortcut() const { return m_ShortcutKey; }
  void SetShortcut(unsigned key) { m_ShortcutKey = key; }
  unsigned GetMinusKey() const { return m_MinusKey; }
  void SetMinusKey(unsigned key) { m_MinusKey = key; };
  bool IsMidiConfigured() const { return m_ShortcutKey; }
  bool operator==(const GOMidiShortcutPattern &other) const;
};

#endif
