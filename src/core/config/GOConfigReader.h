/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGREADER_H
#define GOCONFIGREADER_H

#include <wx/colour.h>
#include <wx/string.h>

class GOConfigReaderDB;

struct IniFileEnumEntry {
  wxString name;
  int value;
};

typedef enum { ODFSetting, CMBSetting } GOSettingType;

class GOConfigReader {
 private:
  bool m_Strict;
  GOConfigReaderDB &m_Config;

  bool Read(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString &value);

 public:
  GOConfigReader(GOConfigReaderDB &cfg, bool strict = true);

  bool ReadBoolean(
    GOSettingType type, wxString group, wxString key, bool required = true);
  bool ReadBoolean(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    bool defaultValue);
  wxColour ReadColor(
    GOSettingType type, wxString group, wxString key, bool required = true);
  wxColour ReadColor(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString defaultValue);
  wxString ReadString(
    GOSettingType type, wxString group, wxString key, bool required = true);
  wxString ReadString(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString defaultValue);
  wxString ReadStringTrim(
    GOSettingType type, wxString group, wxString key, bool required = true);
  wxString ReadStringTrim(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString defaultValue);
  wxString ReadStringNotEmpty(
    GOSettingType type, wxString group, wxString key, bool required = true);
  wxString ReadStringNotEmpty(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString defaultValue);
  int ReadInteger(
    GOSettingType type,
    wxString group,
    wxString key,
    int nmin = 0,
    int nmax = 0,
    bool required = true);
  int ReadInteger(
    GOSettingType type,
    wxString group,
    wxString key,
    int nmin,
    int nmax,
    bool required,
    int defaultValue);
  int ReadLong(
    GOSettingType type,
    wxString group,
    wxString key,
    int nmin = 0,
    int nmax = 0,
    bool required = true);
  int ReadLong(
    GOSettingType type,
    wxString group,
    wxString key,
    int nmin,
    int nmax,
    bool required,
    int defaultValue);
  double ReadFloat(
    GOSettingType type,
    wxString group,
    wxString key,
    double nmin = 0,
    double nmax = 0,
    bool required = true);
  double ReadFloat(
    GOSettingType type,
    wxString group,
    wxString key,
    double nmin,
    double nmax,
    bool required,
    double defaultValue);
  unsigned ReadSize(
    GOSettingType type,
    wxString group,
    wxString key,
    unsigned size_type = 0,
    bool required = true);
  unsigned ReadSize(
    GOSettingType type,
    wxString group,
    wxString key,
    unsigned size_type,
    bool required,
    wxString defaultValue);
  unsigned ReadFontSize(
    GOSettingType type, wxString group, wxString key, bool required = true);
  unsigned ReadFontSize(
    GOSettingType type,
    wxString group,
    wxString key,
    bool required,
    wxString defaultValue);
  int ReadEnum(
    GOSettingType type,
    wxString group,
    wxString key,
    const struct IniFileEnumEntry *entry,
    unsigned count,
    bool required = true);
  int ReadEnum(
    GOSettingType type,
    wxString group,
    wxString key,
    const struct IniFileEnumEntry *entry,
    unsigned count,
    bool required,
    int defaultValue);
};

#endif
