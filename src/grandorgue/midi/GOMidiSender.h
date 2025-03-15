/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDER_H
#define GOMIDISENDER_H

#include <wx/string.h>

#include "midi/GOMidiSenderEventPatternList.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GOMidiSendProxy;
struct IniFileEnumEntry;

class GOMidiSender : public GOMidiSenderEventPatternList {
private:
  static const struct IniFileEnumEntry m_MidiTypes[];
  GOMidiSendProxy &r_proxy;
  int m_ElementID;

public:
  GOMidiSender(GOMidiSendProxy &proxy, GOMidiSenderType type);
  ~GOMidiSender();

  void SetElementID(int id);

  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map);

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
