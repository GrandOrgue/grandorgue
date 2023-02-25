/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGFILEREADER_H
#define GOCONFIGFILEREADER_H

#include <wx/string.h>

#include <map>

class GOOpenedFile;

class GOConfigFileReader {
private:
  std::map<wxString, std::map<wxString, wxString>> m_Entries;
  wxString m_Hash;

  wxString GetNextLine(const wxString &buffer, unsigned &pos);

public:
  GOConfigFileReader();
  ~GOConfigFileReader();

  bool Read(GOOpenedFile *file);
  bool Read(wxString filename);
  wxString GetHash();

  const std::map<wxString, std::map<wxString, wxString>> &GetContent();
  wxString getEntry(wxString group, wxString name);
};

#endif
