/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGWRITER_H
#define GOCONFIGWRITER_H

#include <wx/string.h>

class GOConfigFileWriter;
struct IniFileEnumEntry;

class GOConfigWriter {
private:
  GOConfigFileWriter &m_ConfigFile;
  bool m_Prefix;

public:
  GOConfigWriter(GOConfigFileWriter &cfg, bool prefix);

  void WriteString(wxString group, wxString key, wxString value);
  void WriteInteger(wxString group, wxString key, int value);
  void WriteEnum(
    wxString group,
    wxString key,
    int value,
    const struct IniFileEnumEntry *entry,
    unsigned count);
  void WriteFloat(wxString group, wxString key, float value);
  /**
   * Saves 0 as N, 1 as Y. -1 is not saved at all
   */
  void WriteBooleanTriple(
    const wxString &group, const wxString &key, int value);
  void WriteBoolean(wxString group, wxString key, bool value) {
    WriteBooleanTriple(group, key, (int)value);
  }
};

#endif
