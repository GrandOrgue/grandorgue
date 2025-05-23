/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDER_H
#define GOMIDISENDER_H

#include <wx/string.h>
#include <yaml-cpp/yaml.h>

#include "midi/events/GOMidiSenderEventPatternList.h"

#include "GOMidiElement.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GOMidiSendProxy;

class GOMidiSender : public GOMidiSenderEventPatternList, public GOMidiElement {
private:
  int m_ElementID;
  GOMidiSendProxy *p_proxy;

public:
  GOMidiSender(GOMidiSenderType type);

  int GetElementId() const { return m_ElementID; }
  void SetElementID(int id) { m_ElementID = id; }

  GOMidiSendProxy *GetProxy() const { return p_proxy; }
  void SetProxy(GOMidiSendProxy *pProxy) { p_proxy = pProxy; }

  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const;

  void ToYaml(YAML::Node &yamlNode, GOMidiMap &map) const override;
  void FromYaml(
    const YAML::Node &yamlNode,
    const wxString &yamlPath,
    GOMidiMap &map,
    GOStringSet &usedPaths) override;

  void SetDisplay(bool state);
  void SetKey(unsigned key, unsigned velocity);
  void ResetKey();
  void SetValue(unsigned value);
  void SetLabel(const wxString &text);
  void SetName(const wxString &text);

  static bool hasChannel(GOMidiSenderMessageType type);
  bool HasKey(GOMidiSenderMessageType type) const;
  static bool hasLowValue(GOMidiSenderMessageType type);
  static bool hasHighValue(GOMidiSenderMessageType type);
  static bool hasStart(GOMidiSenderMessageType type);
  static bool hasLength(GOMidiSenderMessageType type);
  static bool isNote(GOMidiSenderMessageType type);
  static unsigned keyLimit(GOMidiSenderMessageType type);
  static unsigned lowValueLimit(GOMidiSenderMessageType type);
  static unsigned highValueLimit(GOMidiSenderMessageType type);
  static unsigned startLimit(GOMidiSenderMessageType type);
  static unsigned lengthLimit(GOMidiSenderMessageType type);
};

#endif
