/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVERBASE_H
#define GOMIDIRECEIVERBASE_H

#include <cstdint>

#include "GOMidiMatchType.h"
#include "GOMidiReceiverEventPatternList.h"
#include "GOTime.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
struct IniFileEnumEntry;

class GOMidiReceiverBase : public GOMidiReceiverEventPatternList {
public:
  constexpr static unsigned KEY_MAP_SIZE = 128;
  using KeyMap = uint8_t[KEY_MAP_SIZE];

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
  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map);
  void PreparePlayback();

  void SetElementID(int id);

  GOMidiMatchType Match(const GOMidiEvent &e);
  GOMidiMatchType Match(const GOMidiEvent &e, int &value);
  GOMidiMatchType Match(
    const GOMidiEvent &e, const KeyMap *pMidiMap, int &key, int &value);

  bool HasDebounce(GOMidiReceiverMessageType type) const;
  static bool hasChannel(GOMidiReceiverMessageType type);
  static bool hasKey(GOMidiReceiverMessageType type);
  bool HasLowKey(GOMidiReceiverMessageType type) const;
  bool HasHighKey(GOMidiReceiverMessageType type) const;
  static bool hasLowerLimit(GOMidiReceiverMessageType type);
  static bool hasUpperLimit(GOMidiReceiverMessageType type);
  static unsigned keyLimit(GOMidiReceiverMessageType type);
  static unsigned lowerValueLimit(GOMidiReceiverMessageType type);
  static unsigned upperValueLimit(GOMidiReceiverMessageType type);
};

#endif
