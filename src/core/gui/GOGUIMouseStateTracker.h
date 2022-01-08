/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMOUSESTATETRACKER_H
#define GOGUIMOUSESTATETRACKER_H

#include <vector>

#include "GOGUIMouseState.h"

class GOGUIMouseStateTracker {
 private:
  std::vector<GOGUIMouseState> m_states;

 public:
  GOGUIMouseState& GetMouseState() { return GetState(this); }

  GOGUIMouseState& GetState(void* id) {
    for (unsigned i = 0; i < m_states.size(); i++)
      if (m_states[i].GetSequence() == id) return m_states[i];

    GOGUIMouseState tmp;
    tmp.SetSequence(id);
    m_states.push_back(tmp);
    return m_states[m_states.size() - 1];
  }

  void ReleaseMouseState() { ReleaseState(this); }

  void ReleaseState(void* id) {
    for (unsigned i = 0; i < m_states.size(); i++)
      if (m_states[i].GetSequence() == id) {
        m_states[i].clear();
        return;
      }
  }
};

#endif
