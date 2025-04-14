/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISHORTCUTPATTERN_H
#define GOMIDISHORTCUTPATTERN_H

class GOMidiShortcutPattern {
public:
  enum ReceiverType {
    KEY_RECV_BUTTON,
    KEY_RECV_ENCLOSURE,
  };
  enum MatchType {
    KEY_MATCH_NONE,
    KEY_MATCH,
    KEY_MATCH_MINUS,
  };

protected:
  ReceiverType m_type;
  unsigned m_ShortcutKey = 0;
  unsigned m_MinusKey = 0;

public:
  GOMidiShortcutPattern(ReceiverType type) : m_type(type) {}
  virtual ~GOMidiShortcutPattern() {}

  ReceiverType GetType() const { return m_type; }

  unsigned GetShortcut() const { return m_ShortcutKey; }
  void SetShortcut(unsigned key) { m_ShortcutKey = key; }
  unsigned GetMinusKey() const { return m_MinusKey; }
  void SetMinusKey(unsigned key) { m_MinusKey = key; };
  bool IsMidiConfigured() const { return m_ShortcutKey; }
  bool operator==(const GOMidiShortcutPattern &other) const;
};

#endif
