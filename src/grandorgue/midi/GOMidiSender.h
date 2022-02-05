/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDER_H
#define GOMIDISENDER_H

#include <wx/string.h>

#include "midi/GOMidiSenderData.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GODefinitionFile;
struct IniFileEnumEntry;

class GOMidiSender : public GOMidiSenderData {
private:
  static const struct IniFileEnumEntry m_MidiTypes[];
  GODefinitionFile *m_organfile;
  int m_ElementID;

public:
  GOMidiSender(GODefinitionFile *organfile, GOMidiSenderType type);
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

  bool HasChannel(GOMidiSendMessageType type);
  bool HasKey(GOMidiSendMessageType type);
  bool HasLowValue(GOMidiSendMessageType type);
  bool HasHighValue(GOMidiSendMessageType type);
  bool HasStart(GOMidiSendMessageType type);
  bool HasLength(GOMidiSendMessageType type);
  unsigned KeyLimit(GOMidiSendMessageType type);
  unsigned LowValueLimit(GOMidiSendMessageType type);
  unsigned HighValueLimit(GOMidiSendMessageType type);
  unsigned StartLimit(GOMidiSendMessageType type);
  unsigned LengthLimit(GOMidiSendMessageType type);

  void Assign(const GOMidiSenderData &data);
};

#endif
