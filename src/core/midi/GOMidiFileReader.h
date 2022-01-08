/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIFILEREADER_H
#define GOMIDIFILEREADER_H

#include <stdint.h>
#include <wx/string.h>

#include "GOBuffer.h"

class GOMidiEvent;
class GOMidiMap;

class GOMidiFileReader {
 private:
  GOMidiMap& m_Map;
  GOBuffer<uint8_t> m_Data;
  unsigned m_Tracks;
  float m_Speed;
  unsigned m_Pos;
  unsigned m_TrackEnd;
  unsigned m_LastStatus;
  unsigned m_CurrentSpeed;
  float m_LastTime;
  unsigned m_PPQ;
  unsigned m_Tempo;

  bool StartTrack();
  unsigned DecodeTime();

 public:
  GOMidiFileReader(GOMidiMap& map);
  ~GOMidiFileReader();

  bool Open(wxString filename);
  bool ReadEvent(GOMidiEvent& e);
  bool Close();
};

#endif
