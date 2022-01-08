/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVERBASE_H
#define GOMIDIRECEIVERBASE_H

#include "GOMidiReceiverData.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
struct IniFileEnumEntry;

class GOMidiReceiverBase : public GOMidiReceiverData {
private:
  typedef struct {
    unsigned device;
    int channel;
    int key;
  } midi_internal_match;

  static const struct IniFileEnumEntry m_MidiTypes[];
  int m_ElementID;
  std::vector<GOTime> m_last;
  std::vector<midi_internal_match> m_Internal;

  MIDI_MATCH_TYPE
  debounce(const GOMidiEvent &e, MIDI_MATCH_TYPE event, unsigned index);
  void deleteInternal(unsigned device);
  unsigned createInternal(unsigned device);

protected:
  virtual void Preconfigure(GOConfigReader &cfg, wxString group);
  virtual int GetTranspose();

public:
  GOMidiReceiverBase(MIDI_RECEIVER_TYPE type);

  virtual void Load(GOConfigReader &cfg, wxString group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, wxString group, GOMidiMap &map);
  void PreparePlayback();

  void SetElementID(int id);

  MIDI_MATCH_TYPE Match(const GOMidiEvent &e);
  MIDI_MATCH_TYPE Match(const GOMidiEvent &e, int &value);
  MIDI_MATCH_TYPE Match(
    const GOMidiEvent &e, const unsigned midi_map[128], int &key, int &value);

  bool HasDebounce(midi_match_message_type type);
  bool HasChannel(midi_match_message_type type);
  bool HasKey(midi_match_message_type type);
  bool HasLowKey(midi_match_message_type type);
  bool HasHighKey(midi_match_message_type type);
  bool HasLowerLimit(midi_match_message_type type);
  bool HasUpperLimit(midi_match_message_type type);
  unsigned KeyLimit(midi_match_message_type type);
  unsigned LowerValueLimit(midi_match_message_type type);
  unsigned UpperValueLimit(midi_match_message_type type);

  virtual void Assign(const GOMidiReceiverData &data);
};

#endif
