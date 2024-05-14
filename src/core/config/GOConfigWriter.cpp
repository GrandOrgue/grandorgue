/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "config/GOConfigWriter.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOUtil.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"

GOConfigWriter::GOConfigWriter(GOConfigFileWriter &cfg, bool prefix)
  : m_ConfigFile(cfg), m_Prefix(prefix) {}

void GOConfigWriter::WriteString(wxString group, wxString key, wxString value) {
  if (m_Prefix)
    m_ConfigFile.AddEntry(wxT('_') + group, key, value);
  else
    m_ConfigFile.AddEntry(group, key, value);
}

void GOConfigWriter::WriteInteger(wxString group, wxString key, int value) {
  wxString str;
  str.Printf(wxT("%d"), value);
  WriteString(group, key, str);
}

void GOConfigWriter::WriteFloat(wxString group, wxString key, float value) {
  wxString str = formatCDDouble(value);
  WriteString(group, key, str);
}

void GOConfigWriter::WriteEnum(
  wxString group,
  wxString key,
  int value,
  const struct IniFileEnumEntry *entry,
  unsigned count) {
  for (unsigned i = 0; i < count; i++)
    if (entry[i].value == value) {
      WriteString(group, key, entry[i].name);
      return;
    }
  wxLogError(
    _("Invalid enum value for /%s/%s: %d"), group.c_str(), key.c_str(), value);
}

void GOConfigWriter::WriteBooleanTriple(
  const wxString &group, const wxString &key, int value) {
  if (value >= 0) {
    wxString str = value ? wxT("Y") : wxT("N");

    WriteString(group, key, str);
  }
}
