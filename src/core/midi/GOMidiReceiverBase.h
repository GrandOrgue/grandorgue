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

  GOMidiMatchType debounce(
    const GOMidiEvent &e, GOMidiMatchType event, unsigned index);
  void deleteInternal(unsigned device);
  unsigned createInternal(unsigned device);

protected:
  virtual void Preconfigure(GOConfigReader &cfg, wxString group);
  virtual int GetTranspose();

public:
  GOMidiReceiverBase(GOMidiReceiverType type);

  virtual void Load(GOConfigReader &cfg, wxString group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, wxString group, GOMidiMap &map);
  void PreparePlayback();

  void SetElementID(int id);

  GOMidiMatchType Match(const GOMidiEvent &e);
  GOMidiMatchType Match(const GOMidiEvent &e, int &value);
  GOMidiMatchType Match(
    const GOMidiEvent &e, const unsigned midi_map[128], int &key, int &value);

  bool HasDebounce(GOMidiReceiveMessageType type);
  bool HasChannel(GOMidiReceiveMessageType type);
  bool HasKey(GOMidiReceiveMessageType type);
  bool HasLowKey(GOMidiReceiveMessageType type);
  bool HasHighKey(GOMidiReceiveMessageType type);
  bool HasLowerLimit(GOMidiReceiveMessageType type);
  bool HasUpperLimit(GOMidiReceiveMessageType type);
  unsigned KeyLimit(GOMidiReceiveMessageType type);
  unsigned LowerValueLimit(GOMidiReceiveMessageType type);
  unsigned UpperValueLimit(GOMidiReceiveMessageType type);

  virtual void Assign(const GOMidiReceiverData &data);
};

#endif
