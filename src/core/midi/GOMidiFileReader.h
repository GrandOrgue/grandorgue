/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIFILEREADER_H
#define GOMIDIFILEREADER_H

#include <map>
#include <stdint.h>

#include <wx/string.h>

#include "GOBuffer.h"

class GOMidiEvent;
class GOMidiMap;

class GOMidiFileReader {
private:
  GOMidiMap &m_Map;
  GOBuffer<uint8_t> m_Data;
  unsigned m_Tracks;
  float m_Speed; // the global speed from the file header
  unsigned m_Pos;
  unsigned m_TrackEnd;
  bool m_IsFirstTrackComplete; // the first track has been read completely
  unsigned m_LastStatus;
  float m_CurrentSpeed; // the current speed for the current track
  float m_CurrentSpeedTo;
  float m_LastTime;
  std::map<float, float>
    m_TempoMap; // maps start time to the speed from the first track
  unsigned m_PPQ;

  bool StartTrack();
  unsigned DecodeTime();

public:
  GOMidiFileReader(GOMidiMap &map);
  ~GOMidiFileReader();

  bool Open(wxString filename);
  bool ReadEvent(GOMidiEvent &e);
  bool Close();
};

#endif
