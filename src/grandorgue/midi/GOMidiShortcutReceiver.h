/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISHORTCUTRECEIVER_H
#define GOMIDISHORTCUTRECEIVER_H

#include <wx/string.h>

#include "GOMidiShortcutPattern.h"

class GOConfigReader;
class GOConfigWriter;

class GOMidiShortcutReceiver : public GOMidiShortcutPattern {
public:
  GOMidiShortcutReceiver(ReceiverType type) : GOMidiShortcutPattern(type) {}

  void Load(GOConfigReader &cfg, wxString group);
  void Save(GOConfigWriter &cfg, wxString group);

  MatchType Match(unsigned key);

  bool RenewFrom(const GOMidiShortcutPattern &newPattern);
};

#endif
