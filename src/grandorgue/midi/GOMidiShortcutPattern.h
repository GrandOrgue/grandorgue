/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISHORTCUTPATTERN_H
#define GOMIDISHORTCUTPATTERN_H

typedef enum {
  KEY_RECV_BUTTON,
  KEY_RECV_ENCLOSURE,
} KEY_RECEIVER_TYPE;

typedef enum {
  KEY_MATCH_NONE,
  KEY_MATCH,
  KEY_MATCH_MINUS,
} KEY_MATCH_TYPE;

class GOMidiShortcutPattern {
protected:
  KEY_RECEIVER_TYPE m_type;
  unsigned m_ShortcutKey = 0;
  unsigned m_MinusKey = 0;

public:
  GOMidiShortcutPattern(KEY_RECEIVER_TYPE type) : m_type(type) {}
  virtual ~GOMidiShortcutPattern() {}

  KEY_RECEIVER_TYPE GetType() const { return m_type; }

  unsigned GetShortcut() const { return m_ShortcutKey; }
  void SetShortcut(unsigned key) { m_ShortcutKey = key; }
  unsigned GetMinusKey() const { return m_MinusKey; }
  void SetMinusKey(unsigned key) { m_MinusKey = key; };
  bool operator==(const GOMidiShortcutPattern &other) const;
};

#endif
