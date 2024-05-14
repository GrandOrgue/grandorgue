/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVERBASE_H
#define GOMIDIRECEIVERBASE_H

#include "GOMidiMatchType.h"
#include "GOMidiReceiverEventPatternList.h"
#include "GOTime.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
struct IniFileEnumEntry;

class GOMidiReceiverBase : public GOMidiReceiverEventPatternList {
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

  virtual void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, wxString group, GOMidiMap &map);
  void PreparePlayback();

  void SetElementID(int id);

  GOMidiMatchType Match(const GOMidiEvent &e);
  GOMidiMatchType Match(const GOMidiEvent &e, int &value);
  GOMidiMatchType Match(
    const GOMidiEvent &e, const unsigned midi_map[128], int &key, int &value);

  bool HasDebounce(GOMidiReceiverMessageType type);
  bool HasChannel(GOMidiReceiverMessageType type);
  bool HasKey(GOMidiReceiverMessageType type);
  bool HasLowKey(GOMidiReceiverMessageType type);
  bool HasHighKey(GOMidiReceiverMessageType type);
  bool HasLowerLimit(GOMidiReceiverMessageType type);
  bool HasUpperLimit(GOMidiReceiverMessageType type);
  unsigned KeyLimit(GOMidiReceiverMessageType type);
  unsigned LowerValueLimit(GOMidiReceiverMessageType type);
  unsigned UpperValueLimit(GOMidiReceiverMessageType type);
};

#endif
