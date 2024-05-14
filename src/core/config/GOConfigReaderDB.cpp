/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "config/GOConfigReaderDB.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigFileReader.h"

GOConfigReaderDB::GOConfigReaderDB(bool case_sensitive)
  : m_CaseSensitive(case_sensitive), m_ODF(1000), m_ODF_LC(1000), m_CMB(100) {}

GOConfigReaderDB::~GOConfigReaderDB() {}

void GOConfigReaderDB::ReportUnused() {
  for (GOBoolHashMap::iterator i = m_CMBUsed.begin(); i != m_CMBUsed.end();
       i++) {
    if (!i->second) {
      wxLogWarning(_("Unused CMB entry '%s'"), i->first.c_str());
    }
  }
  bool warn_old = false;
  for (GOBoolHashMap::iterator i = m_ODFUsed.begin(); i != m_ODFUsed.end();
       i++) {
    if (!i->second) {
      if (i->first.StartsWith(wxT("_"))) {
        if (!warn_old) {
          wxLogWarning(_("Old GO 0.2 styled setting in ODF"));
        }
        warn_old = true;
      } else {
        wxLogWarning(_("Unused ODF entry '%s'"), i->first.c_str());
      }
    }
  }
}

bool GOConfigReaderDB::ReadData(
  GOConfigFileReader &ODF, GOSettingType type, bool handle_prefix) {
  const std::map<wxString, std::map<wxString, wxString>> &entries
    = ODF.GetContent();
  bool changed = false;

  for (std::map<wxString, std::map<wxString, wxString>>::const_iterator i
       = entries.begin();
       i != entries.end();
       i++) {
    const std::map<wxString, wxString> &g = i->second;
    wxString group = i->first;

    if (!handle_prefix || group.StartsWith(wxT("_"))) {
      if (handle_prefix)
        group = group.Mid(1);

      for (std::map<wxString, wxString>::const_iterator j = g.begin();
           j != g.end();
           j++) {
        const wxString &key = j->first;
        const wxString &value = j->second;
        wxString k = group + wxT('/') + key;

        if (type == ODFSetting) {
          AddEntry(m_ODF, k, value);
          if (!m_CaseSensitive)
            AddEntry(m_ODF_LC, k.Lower(), value);
          m_ODFUsed[k] = false;
        } else {
          AddEntry(m_CMB, k, value);
          m_CMBUsed[k] = false;
        }
        changed = true;
      }
    }
  }

  return changed;
}

void GOConfigReaderDB::ClearCMB() {
  m_CMB.clear();
  m_CMBUsed.clear();
}

void GOConfigReaderDB::AddEntry(
  GOStringHashMap &hash, wxString key, wxString value) {
  GOStringHashMap::iterator i = hash.find(key);
  if (i != hash.end()) {
    wxLogWarning(_("Duplicate entry: %s"), key.c_str());
  }
  hash[key] = value;
}

bool GOConfigReaderDB::GetString(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  wxString &value) {
  wxString index = group + wxT("/") + key;
  if (type == CMBSetting) {
    m_CMBUsed[index] = true;
    GOStringHashMap::iterator i = m_CMB.find(index);
    if (i != m_CMB.end()) {
      value = i->second;
      return true;
    }
  }
  if (type == ODFSetting) {
    m_ODFUsed[index] = true;
    GOStringHashMap::iterator i = m_ODF.find(index);
    if (i != m_ODF.end()) {
      value = i->second;
      return true;
    }
  }
  if (type == ODFSetting && !m_CaseSensitive) {
    GOStringHashMap::iterator i = m_ODF_LC.find(index.Lower());
    if (i != m_ODF.end()) {
      wxLogWarning(
        _("Incorrect case for section '%s' entry '%s'"),
        group.c_str(),
        key.c_str());
      value = i->second;
      return true;
    }
  }
  return false;
}
