/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGFILEWRITER_H
#define GOCONFIGFILEWRITER_H

#include <cstdint>
#include <wx/string.h>

#include <map>

template <class T> class GOBuffer;

class GOConfigFileWriter {
private:
  std::map<wxString, std::map<wxString, wxString>> m_Entries;

public:
  GOConfigFileWriter();

  void AddEntry(wxString group, wxString name, wxString value);
  bool GetFileContent(GOBuffer<uint8_t> &buffer);
  bool Save(wxString filename);
};

#endif
