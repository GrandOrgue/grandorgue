/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISHORTCUTRECEIVER_H
#define GOMIDISHORTCUTRECEIVER_H

#include <wx/string.h>

#include "midi/events/GOMidiShortcutPattern.h"

#include "GOMidiElement.h"

class GOConfigReader;
class GOConfigWriter;

class GOMidiShortcutReceiver : public GOMidiShortcutPattern,
                               public GOMidiElement {
public:
  GOMidiShortcutReceiver(ReceiverType type) : GOMidiShortcutPattern(type) {}

  void Load(GOConfigReader &cfg, const wxString &group);
  void Save(GOConfigWriter &cfg, const wxString &group) const;

  void ToYaml(YAML::Node &yamlNode, GOMidiMap &map) const override;
  void FromYaml(
    const YAML::Node &yamlNode,
    const wxString &path,
    GOMidiMap &map,
    GOStringSet &usedPaths) override;

  MatchType Match(unsigned key);

  bool RenewFrom(const GOMidiShortcutPattern &newPattern);
};

#endif
