/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

  void Load(GOConfigReader &cfg, wxString group, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, wxString group, GOMidiMap &map);

  void SetDisplay(bool state);
  void SetKey(unsigned key, unsigned velocity);
  void ResetKey();
  void SetValue(unsigned value);
  void SetLabel(const wxString &text);
  void SetName(const wxString &text);

  bool HasChannel(GOMidiSenderMessageType type);
  bool HasKey(GOMidiSenderMessageType type);
  bool HasLowValue(GOMidiSenderMessageType type);
  bool HasHighValue(GOMidiSenderMessageType type);
  bool HasStart(GOMidiSenderMessageType type);
  bool HasLength(GOMidiSenderMessageType type);
  bool IsNote(GOMidiSenderMessageType type);
  unsigned KeyLimit(GOMidiSenderMessageType type);
  unsigned LowValueLimit(GOMidiSenderMessageType type);
  unsigned HighValueLimit(GOMidiSenderMessageType type);
  unsigned StartLimit(GOMidiSenderMessageType type);
  unsigned LengthLimit(GOMidiSenderMessageType type);
};

#endif
