/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGREADERDB_H
#define GOCONFIGREADERDB_H

#include <wx/hashmap.h>
#include <wx/string.h>

#include "config/GOConfigReader.h"

class GOConfigFileReader;

class GOConfigReaderDB {
private:
  WX_DECLARE_STRING_HASH_MAP(wxString, GOStringHashMap);
  WX_DECLARE_STRING_HASH_MAP(bool, GOBoolHashMap);

  bool m_CaseSensitive;
  GOStringHashMap m_ODF;
  GOStringHashMap m_ODF_LC;
  GOStringHashMap m_CMB;
  GOBoolHashMap m_ODFUsed;
  GOBoolHashMap m_CMBUsed;

  void AddEntry(GOStringHashMap &hash, wxString key, wxString value);

public:
  GOConfigReaderDB(bool case_sensitive = true);
  ~GOConfigReaderDB();
  void ReportUnused();
  void ClearCMB();
  bool ReadData(
    GOConfigFileReader &ODF, GOSettingType type, bool handle_prefix);

  bool GetString(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    wxString &value);
};

#endif
