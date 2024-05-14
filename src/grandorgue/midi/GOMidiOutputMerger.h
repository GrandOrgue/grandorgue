/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOUTPUTMERGER_H
#define GOMIDIOUTPUTMERGER_H

#include <wx/string.h>

#include <cstdint>
#include <vector>

#include "midi/GOMidiEvent.h"

class GOMidiOutputMerger {
private:
  struct GOMidiOutputMergerHWState {
    GOMidiEvent::MidiType type;
    int key;
    wxString content;
  };
  std::vector<GOMidiOutputMergerHWState> m_HWState;
  std::vector<std::vector<uint8_t>> m_RodgersState;

public:
  GOMidiOutputMerger();

  void Clear();
  bool Process(GOMidiEvent &e);
};

#endif
