/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiFileReader.h"

#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "GOMidiEvent.h"
#include "GOMidiFile.h"

GOMidiFileReader::GOMidiFileReader(GOMidiMap &map)
  : m_Map(map),
    m_Data(),
    m_Tracks(0),
    m_Speed(0),
    m_Pos(0),
    m_TrackEnd(0),
    m_IsFirstTrackComplete(false),
    m_LastStatus(0),
    m_CurrentSpeed(0),
    m_CurrentSpeedTo(-1),
    m_LastTime(0),
    m_PPQ(0) {}

GOMidiFileReader::~GOMidiFileReader() { m_Data.free(); }

bool GOMidiFileReader::Open(wxString filename) {
  m_Data.free();

  wxFile file;
  if (!file.Open(filename, wxFile::read)) {
    wxLogError(_("Failed to open %s"), filename.c_str());
    return false;
  }
  try {
    m_Data.resize(file.Length());
  } catch (GOOutOfMemory e) {
    wxLogError(_("Out of memory"));
    return false;
  }
  if (
    file.Read(m_Data.get(), m_Data.GetCount()) != (ssize_t)m_Data.GetCount()) {
    wxLogError(_("Failed to read content of %s"), filename.c_str());
    return false;
  }
  if (m_Data.GetCount() < sizeof(MIDIHeaderChunk)) {
    wxLogError(_("MIDI header missing"));
    return false;
  }
  m_Pos = sizeof(MIDIHeaderChunk);
  m_TrackEnd = 0;
  MIDIHeaderChunk *h = (MIDIHeaderChunk *)m_Data.get();
  if (
    memcmp(h->header.type, "MThd", sizeof(h->header.type))
    || h->header.len != 6) {
    wxLogError(_("Malformed MIDI header"));
    return false;
  }
  m_Tracks = h->tracks;
  switch (h->type) {
  case 0:
    if (m_Tracks != 1) {
      wxLogError(_("MIDI file type 0 only supports one track"));
      return false;
    }
    break;

  case 1:
    if (m_Tracks < 1) {
      wxLogError(_("MIDI file type 1 has not tracks"));
      return false;
    }
    break;

  case 2:
    wxLogError(_("MIDI file type 2 is not supported"));
    return false;

  default:
    wxLogError(_("Unknown MIDI file type %d"), (int)h->type);
    return false;
  }

  unsigned tempo = 0x7A120; // 120 BPM
  unsigned ppq = h->ppq;

  if (ppq & 0x8000) {
    unsigned frames = 1 + ((-ppq >> 8) & 0x7F);
    unsigned res = ppq & 0xFF;
    m_Speed = 1000.0 / frames / (res ? res : 1);
    m_PPQ = 0;
  } else {
    m_PPQ = ppq;
    m_Speed = tempo / 1000.0 / (m_PPQ ? m_PPQ : 1);
  }
  m_TempoMap.clear();
  return true;
}

bool GOMidiFileReader::StartTrack() {
  do {
    if (m_Pos >= m_Data.GetCount())
      return false;
    if (m_Pos + sizeof(MIDIFileHeader) > m_Data.GetCount()) {
      wxLogError(_("Incomplete chunk at offset %d"), m_Pos);
      m_Pos += sizeof(MIDIFileHeader);
      return false;
    }
    MIDIFileHeader *h = (MIDIFileHeader *)&m_Data[m_Pos];
    if (memcmp(h->type, "MTrk", sizeof(h->type))) {
      wxLogError(_("Not recognized MIDI chunk at offset %d"), m_Pos);
      m_Pos += sizeof(MIDIFileHeader) + h->len;
      if (m_Pos >= m_Data.GetCount()) {
        wxLogError(_("Incomplete chunk"));
        return false;
      }
    } else {
      m_TrackEnd = m_Pos + sizeof(MIDIFileHeader) + h->len;
      if (m_Pos >= m_Data.GetCount()) {
        wxLogError(_("Incomplete chunk"));
        return false;
      }
      if (!m_Tracks)
        wxLogError(_("Incorrect track count"));
      else
        m_Tracks--;
      m_Pos += sizeof(MIDIFileHeader);
      m_LastStatus = 0;
      m_LastTime = 0;
      m_CurrentSpeed = m_Speed;
      m_CurrentSpeedTo = -1;
      if (m_IsFirstTrackComplete) {
        // find the first tempo change from the tempo map
        auto mapIter = m_TempoMap.lower_bound(0);

        if (mapIter != m_TempoMap.end())
          m_CurrentSpeedTo = mapIter->first;
      }
      return true;
    }
  } while (true);
}

unsigned GOMidiFileReader::DecodeTime() {
  unsigned time = 0;
  if (m_Pos >= m_TrackEnd) {
    wxLogError(_("Incomplete timestamp at offset %d"), m_Pos);
    return 0;
  }
  uint8_t c = m_Data[m_Pos++];
  while ((c & 0x80)) {
    time = (time << 7) | (c & 0x7F);
    if (m_Pos >= m_TrackEnd) {
      wxLogError(_("Incomplete timestamp at offset %d"), m_Pos);
      return 0;
    }
    c = m_Data[m_Pos++];
  }
  time = (time << 7) | (c & 0x7F);
  return time;
}

bool GOMidiFileReader::ReadEvent(GOMidiEvent &e) {
  do {
    std::vector<unsigned char> msg;
    if (m_Pos >= m_Data.GetCount())
      return false;
    if (m_TrackEnd && m_Pos >= m_TrackEnd) {
      wxLogError(_("End of track marker missing at offset %d"), m_Pos);
      m_Pos = m_TrackEnd;
      m_TrackEnd = 0;
      m_IsFirstTrackComplete = true;
    }
    if (!m_TrackEnd)
      if (!StartTrack())
        return false;
    unsigned rel_time = DecodeTime();
    if (m_Pos >= m_TrackEnd)
      return false;
    unsigned len;
    if (m_Data[m_Pos] & 0x80) {
      msg.push_back(m_Data[m_Pos]);
      m_Pos++;
    } else if (m_LastStatus)
      msg.push_back(m_LastStatus);
    else {
      wxLogError(_("Status byte missing at offset %d"), m_Pos);
      return false;
    }
    m_LastStatus = 0;
    switch (msg[0] & 0xF0) {
    case 0x80:
    case 0x90:
    case 0xA0:
    case 0xB0:
    case 0xE0:
      m_LastStatus = msg[0];
      len = 2;
      break;

    case 0xC0:
    case 0xD0:
      m_LastStatus = msg[0];
      len = 1;
      break;

    case 0xF0:
      switch (msg[0]) {
      case 0xF6:
      case 0xF8:
      case 0xF9:
      case 0xFA:
      case 0xFB:
      case 0xFC:
      case 0xFE:
        len = 0;
        break;

      case 0xF1:
      case 0xF3:
        len = 1;
        break;
      case 0xF2:
        len = 2;
        break;

      case 0xF0:
      case 0xF7:
        if (m_Pos + 1 > m_TrackEnd) {
          wxLogError(_("Incomplete MIDI message at %d"), m_Pos - 1);
          return false;
        }
        len = m_Data[m_Pos];
        m_Pos += 1;
        break;

      case 0xFF:
        if (m_Pos + 2 > m_TrackEnd) {
          wxLogError(_("Incomplete MIDI message at %d"), m_Pos - 1);
          return false;
        }
        msg.push_back(m_Data[m_Pos]);
        msg.push_back(m_Data[m_Pos + 1]);
        len = m_Data[m_Pos + 1];
        m_Pos += 2;
        break;

      default:
        wxLogError(_("Unknown MIDI message %02X at %d"), msg[0], m_Pos - 1);
        return false;
      }
      break;

    default:
      wxLogError(_("Unknown MIDI message %02X at %d"), msg[0], m_Pos - 1);
      return false;
    }
    if (m_Pos + len > m_TrackEnd) {
      wxLogError(_("Incomplete MIDI message at %d"), m_Pos - msg.size());
      return false;
    }
    for (unsigned i = 0; i < len; i++)
      msg.push_back(m_Data[m_Pos++]);
    m_LastTime += rel_time * m_CurrentSpeed;

    if (msg[0] == 0xFF && msg[1] == 0x2F && msg[2] == 0x00) {
      if (m_Pos != m_TrackEnd)
        wxLogError(_("Events after end of track marker at %d"), m_Pos);

      m_Pos = m_TrackEnd;
      m_TrackEnd = 0;
      m_IsFirstTrackComplete = true;
      continue;
    }

    // Look up the tempo map: should we change the tempo?
    if (
      m_IsFirstTrackComplete && m_CurrentSpeedTo >= 0
      && m_LastTime >= m_CurrentSpeedTo) {
      // inherit tempo changes from the first track
      // find the next tempo from m_TempoMap
      auto mapIter = m_TempoMap.lower_bound(m_CurrentSpeedTo);

      assert(mapIter != m_TempoMap.end()); // due the rule for m_CurrentSpeedTo
      if (mapIter->first > m_LastTime)     // not yet
        mapIter--;

      // now mapIter points to the greatest entry <= m_LastTime
      m_CurrentSpeed = mapIter->second;
      mapIter++; // to the next tempo change
      m_CurrentSpeedTo = mapIter != m_TempoMap.end() ? mapIter->first : -1;
    }

    // try to process tempo changes from the current track of the midi file
    if (msg[0] == 0xFF && msg[1] == 0x51 && msg[2] == 0x03) {
      unsigned tempo = (msg[3] << 16) | (msg[4] << 8) | (msg[5]);

      m_CurrentSpeed = tempo / 1000.0 / (m_PPQ ? m_PPQ : 1);

      if (!m_IsFirstTrackComplete) {
        // fill out the tempo map
        m_TempoMap[m_LastTime] = m_CurrentSpeed;
      }
      continue;
    }

    e.FromMidi(msg, m_Map);
    e.SetTime(m_LastTime);
    if (e.GetMidiType() != GOMidiEvent::MIDI_NONE)
      return true;
  } while (true);
}

bool GOMidiFileReader::Close() {
  if (m_Tracks)
    wxLogError(_("Some tracks are missing"));
  if (m_TrackEnd)
    wxLogError(_("Last track is missing the end marker"));
  if (m_Pos < m_TrackEnd) {
    wxLogError(_("Error decoding a track"));
    return false;
  }
  if (m_Pos < m_Data.GetCount()) {
    wxLogError(_("Garbage detected in the MIDI file"));
    return false;
  }
  return true;
}
