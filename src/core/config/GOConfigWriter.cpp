/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "config/GOConfigWriter.h"

#include <format>

#include <wx/intl.h>
#include <wx/log.h>

#include "GOConfigEnum.h"
#include "GOConfigFileWriter.h"

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
  std::string str = std::format("{:.6f}", value);
  WriteString(group, key, str);
}

void GOConfigWriter::WriteEnum(
  const wxString &group,
  const wxString &key,
  const GOConfigEnum &configEnum,
  int value) {
  const wxString &valueName = configEnum.GetName(value);

  if (valueName.IsEmpty())
    wxLogError(_("Invalid enum value for /%s/%s: %d"), group, key, value);
  else
    WriteString(group, key, valueName);
}

void GOConfigWriter::WriteBooleanTriple(
  const wxString &group, const wxString &key, int value) {
  if (value >= 0) {
    wxString str = value ? wxT("Y") : wxT("N");

    WriteString(group, key, str);
  }
}
