/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVER_H
#define GOMIDIRECEIVER_H

#include <cstdint>

#include "config/GOConfigEnum.h"
#include "midi/events/GOMidiMatchType.h"
#include "midi/events/GOMidiReceiverEventPatternList.h"

#include "GOMidiElement.h"
#include "GOTime.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;

class GOMidiReceiver : public GOMidiReceiverEventPatternList,
                       public GOMidiElement {
public:
  static const GOConfigEnum RECEIVER_TYPES;
  static constexpr unsigned KEY_MAP_SIZE = 128;
  using KeyMap = uint8_t[KEY_MAP_SIZE];

private:
  typedef struct {
    unsigned device;
    int channel;
    int key;
  } midi_internal_match;

  int m_ElementID;
  std::vector<GOTime> m_last;
  std::vector<midi_internal_match> m_Internal;

  GOMidiReceiverMessageType GetDefaultMidiType() const;

  GOMidiMatchType debounce(
    const GOMidiEvent &e, GOMidiMatchType event, unsigned index);
  void deleteInternal(unsigned device);
  unsigned createInternal(unsigned device);

public:
  GOMidiReceiver(GOMidiReceiverType type);

  void Load(
    bool isOdfCheck,
    GOConfigReader &cfg,
    const wxString &group,
    GOMidiMap &map);
  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const;

  void ToYaml(YAML::Node &yamlNode, GOMidiMap &map) const override;
  void FromYaml(
    const YAML::Node &yamlNode,
    const wxString &yamlPath,
    GOMidiMap &map,
    GOStringSet &usedPaths) override;

public:
  void PreparePlayback();

  void SetElementID(int id) { m_ElementID = id; }

  GOMidiMatchType Match(const GOMidiEvent &e);
  GOMidiMatchType Match(
    const GOMidiEvent &e,
    const KeyMap *pMidiMap,
    int transpose,
    int &key,
    int &value);

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
