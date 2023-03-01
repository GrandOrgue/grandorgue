/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMOUSESTATE_H
#define GOGUIMOUSESTATE_H

class GOGUIMouseState {
  void *m_sequence;
  void *m_control;
  unsigned m_index;

public:
  GOGUIMouseState() {
    m_sequence = 0;
    clear();
  }

  void clear() {
    m_control = 0;
    m_index = 0;
  }

  void *GetSequence() { return m_sequence; }

  void *GetControl() { return m_control; }

  unsigned GetIndex() { return m_index; }

  void SetSequence(void *sequence) { m_sequence = sequence; }

  void SetControl(void *control) { m_control = control; }

  void SetIndex(unsigned index) { m_index = index; }
};

#endif
