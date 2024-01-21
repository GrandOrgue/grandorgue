/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGREADER_H
#define GOCONFIGREADER_H

#include <unordered_set>

#include <wx/hashmap.h>
#include <wx/string.h>

#include "GOBool3.h"
#include "GOLogicalColour.h"

class GOConfigReaderDB;

struct IniFileEnumEntry {
  wxString name;
  int value;
};

typedef enum { ODFSetting, CMBSetting } GOSettingType;

class GOConfigReader {
private:
  using StringSet = std::unordered_set<wxString, wxStringHash, wxStringEqual>;

  GOConfigReaderDB &m_Config;
  StringSet m_GroupsInUse;
  bool m_Strict;
  bool m_IsHw1Check;

  bool Read(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    wxString &value);

public:
  GOConfigReader(
    GOConfigReaderDB &cfg, bool strict = true, bool hw1Check = false);

  /**
   * Reads a triple-value boolean (-1 - not defined, 0 - false, 1 - true)
   */
  GOBool3 ReadBooleanTriple(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required);
  bool ReadBoolean(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  bool ReadBoolean(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    bool defaultValue);
  GOLogicalColour ReadColor(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  GOLogicalColour ReadColor(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    const wxString &defaultValue);
  wxString ReadString(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  wxString ReadString(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    const wxString &defaultValue);
  wxString ReadStringTrim(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  wxString ReadStringTrim(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    const wxString &defaultValue);
  wxString ReadStringNotEmpty(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  wxString ReadStringNotEmpty(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    const wxString &defaultValue);
  wxString ReadFileName(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  int ReadInteger(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    int nmin = 0,
    int nmax = 0,
    bool required = true);
  int ReadInteger(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    int nmin,
    int nmax,
    bool required,
    int defaultValue);
  GOBool3 ReadBool3FromInt(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required) {
    return to_bool3(ReadInteger(
      type, group, key, BOOL3_MIN, BOOL3_MAX, required, BOOL3_DEFAULT));
  }
  int ReadLong(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    int nmin = 0,
    int nmax = 0,
    bool required = true);
  int ReadLong(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    int nmin,
    int nmax,
    bool required,
    int defaultValue);
  double ReadFloat(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    double nmin = 0,
    double nmax = 0,
    bool required = true);
  double ReadFloat(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    double nmin,
    double nmax,
    bool required,
    double defaultValue);
  unsigned ReadSize(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    unsigned size_type = 0,
    bool required = true);
  unsigned ReadSize(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    unsigned size_type,
    bool required,
    const wxString &defaultValue);
  unsigned ReadFontSize(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required = true);
  unsigned ReadFontSize(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    bool required,
    const wxString &defaultValue);
  int ReadEnum(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    const struct IniFileEnumEntry *entry,
    unsigned count,
    bool required = true);
  int ReadEnum(
    GOSettingType type,
    const wxString &group,
    const wxString &key,
    const struct IniFileEnumEntry *entry,
    unsigned count,
    bool required,
    int defaultValue);

  /**
   * Mark the group as used. Throws an exception if the group has already been
   * marked as in use
   * @param group the group name
   */
  void MarkGroupInUse(const wxString &group);
};

#endif
