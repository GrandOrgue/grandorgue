/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPLAYERCONTENT_H
#define GOMIDIPLAYERCONTENT_H

#include <wx/string.h>

#include <vector>

class GOMidiEvent;
class GOMidiMap;
class GOMidiFileReader;

class GOMidiPlayerContent {
private:
  std::vector<GOMidiEvent> m_Events;
  unsigned m_Pos;

  void ReadFileContent(
    GOMidiFileReader &reader, std::vector<GOMidiEvent> &events);
  void SetupManual(GOMidiMap &map, unsigned channel, wxString ID);

public:
  GOMidiPlayerContent();
  virtual ~GOMidiPlayerContent();

  void Clear();
  void Reset();
  bool IsLoaded();
  bool Load(
    GOMidiFileReader &reader, GOMidiMap &map, unsigned manuals, bool pedal);

  const GOMidiEvent &GetCurrentEvent();
  bool Next();
};

#endif
