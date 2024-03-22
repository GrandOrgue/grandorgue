/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "config/GOConfigReader.h"

#include <unordered_set>
#include <wx/intl.h>
#include <wx/log.h>

#include "GOBool3.h"
#include "GOConfigReaderDB.h"
#include "GOUtil.h"

GOConfigReader::GOConfigReader(
  GOConfigReaderDB &cfg, bool strict, bool hw1Check)
  : m_Config(cfg), m_Strict(strict), m_IsHw1Check(hw1Check) {}

bool GOConfigReader::Read(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  wxString &value) {
  bool found = false;

  if (required) {
    if (group.length() >= 6 && group.Mid(0, 6) == wxT("Setter")) // Setter
                                                                 // groups
                                                                 // aren't
                                                                 // required.
      required = false;
    else if (group.length() >= 5 && group.Mid(0, 5) == wxT("Panel")) {
      if (group.length() >= 14 && group.Mid(8, 6) == wxT("Setter")) // Setter
                                                                    // groups
                                                                    // aren't
                                                                    // required.
        required = false;
    } else if (
      group.length() >= 12
      && group.Mid(0, 12)
        == wxT("FrameGeneral")) // FrameGeneral groups aren't required.
      required = false;
  }

  found = m_Config.GetString(type, group, key, value);

  if (!found) {
    if (required) {
      wxString error;
      error.Printf(
        _("Missing required value section '%s' entry '%s'"),
        group.c_str(),
        key.c_str());
      throw error;
    } else
      return false;
  }
  return true;
}

wxString GOConfigReader::ReadString(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  const wxString &defaultValue) {
  wxString value;
  bool found = Read(type, group, key, required, value);

  if (!found)
    value = defaultValue;
  return value;
}

wxString GOConfigReader::ReadString(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadString(type, group, key, required, wxT(""));
}

wxString GOConfigReader::ReadStringTrim(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  const wxString &defaultValue) {
  wxString value = ReadString(type, group, key, required, defaultValue);

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  return value;
}

wxString GOConfigReader::ReadStringTrim(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadStringTrim(type, group, key, required, wxT(""));
}

wxString GOConfigReader::ReadStringNotEmpty(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  const wxString &defaultValue) {
  wxString value = ReadString(type, group, key, required, defaultValue);

  if (value.Trim().length() == 0 && m_Strict) {
    wxLogWarning(
      _("Empty at section '%s' entry '%s'"), group.c_str(), key.c_str());
  }
  return value;
}

wxString GOConfigReader::ReadStringNotEmpty(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadStringNotEmpty(type, group, key, required, wxT(""));
}

wxString GOConfigReader::ReadFileName(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  const wxString fileName = ReadStringTrim(type, group, key, required);

  if (m_IsHw1Check && fileName.Find(wxT('/')) != wxNOT_FOUND) {
    wxLogWarning(
      _("Filename '%s' contains non-portable directory separator /"), fileName);
  }
  return fileName;
}

bool GOConfigReader::ReadBoolean(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadBoolean(type, group, key, required, false);
}

GOBool3 GOConfigReader::ReadBooleanTriple(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  wxString value;

  if (!Read(type, group, key, required, value))
    return BOOL3_DEFAULT;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  if (value == wxT("Y") || value == wxT("y"))
    return BOOL3_TRUE;
  if (value == wxT("N") || value == wxT("n"))
    return BOOL3_FALSE;
  value.MakeUpper();
  wxLogWarning(
    _("Strange boolean value for section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  if (value.Length() && value[0] == wxT('Y'))
    return BOOL3_TRUE;
  else if (value.Length() && value[0] == wxT('N'))
    return BOOL3_FALSE;

  wxString error;
  error.Printf(
    _("Invalid boolean value at section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  throw error;
}

bool GOConfigReader::ReadBoolean(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  bool defaultValue) {
  return to_bool(ReadBooleanTriple(type, group, key, required), defaultValue);
}

GOLogicalColour GOConfigReader::ReadColor(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadColor(type, group, key, required, wxT("BLACK"));
}

bool parseColor(GOLogicalColour &result, wxString value) {
  if (value.length() == 7 && value[0] == wxT('#')) {
    unsigned r = 0, g = 0, b = 0;

    if (wxT('0') <= value[1] && value[1] <= wxT('9'))
      r = r * 10 + (value[1] - wxT('0'));
    else if (wxT('A') <= value[1] && value[1] <= wxT('F'))
      r = r * 10 + (value[1] - wxT('A')) + 10;
    else
      return false;
    if (wxT('0') <= value[2] && value[2] <= wxT('9'))
      r = r * 10 + (value[2] - wxT('0'));
    else if (wxT('A') <= value[2] && value[2] <= wxT('F'))
      r = r * 10 + (value[2] - wxT('A')) + 10;
    else
      return false;

    if (wxT('0') <= value[3] && value[3] <= wxT('9'))
      g = g * 10 + (value[3] - wxT('0'));
    else if (wxT('A') <= value[3] && value[3] <= wxT('F'))
      g = g * 10 + (value[3] - wxT('A')) + 10;
    else
      return false;
    if (wxT('0') <= value[4] && value[4] <= wxT('9'))
      g = g * 10 + (value[4] - wxT('0'));
    else if (wxT('A') <= value[4] && value[4] <= wxT('F'))
      g = g * 10 + (value[4] - wxT('A')) + 10;
    else
      return false;

    if (wxT('0') <= value[5] && value[5] <= wxT('9'))
      b = b * 10 + (value[5] - wxT('0'));
    else if (wxT('A') <= value[5] && value[5] <= wxT('F'))
      b = b * 10 + (value[5] - wxT('A')) + 10;
    else
      return false;
    if (wxT('0') <= value[6] && value[6] <= wxT('9'))
      b = b * 10 + (value[6] - wxT('0'));
    else if (wxT('A') <= value[6] && value[6] <= wxT('F'))
      b = b * 10 + (value[6] - wxT('A')) + 10;
    else
      return false;

    result.m_red = r;
    result.m_green = g;
    result.m_blue = b;
    return true;
  }
  return false;
}

GOLogicalColour GOConfigReader::ReadColor(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  const wxString &defaultValue) {
  wxString value;
  if (!Read(type, group, key, required, value))
    value = defaultValue;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  value.MakeUpper();

  if (value == wxT("BLACK"))
    return GOLogicalColour::BLACK;
  else if (value == wxT("BLUE"))
    return GOLogicalColour::BLUE;
  else if (value == wxT("DARK BLUE"))
    return GOLogicalColour::DARK_BLUE;
  else if (value == wxT("GREEN"))
    return GOLogicalColour::GREEN;
  else if (value == wxT("DARK GREEN"))
    return GOLogicalColour::DARK_GREEN;
  else if (value == wxT("CYAN"))
    return GOLogicalColour::CYAN;
  else if (value == wxT("DARK CYAN"))
    return GOLogicalColour::DARK_CYAN;
  else if (value == wxT("RED"))
    return GOLogicalColour::RED;
  else if (value == wxT("DARK RED"))
    return GOLogicalColour::DARK_RED;
  else if (value == wxT("MAGENTA"))
    return GOLogicalColour::MAGENTA;
  else if (value == wxT("DARK MAGENTA"))
    return GOLogicalColour::DARK_MAGENTA;
  else if (value == wxT("YELLOW"))
    return GOLogicalColour::YELLOW;
  else if (value == wxT("DARK YELLOW"))
    return GOLogicalColour::DARK_YELLOW;
  else if (value == wxT("LIGHT GREY"))
    return GOLogicalColour::LIGHT_GREY;
  else if (value == wxT("DARK GREY"))
    return GOLogicalColour::DARK_GREY;
  else if (value == wxT("WHITE"))
    return GOLogicalColour::WHITE;
  else if (value == wxT("BROWN"))
    return GOLogicalColour::BROWN;

  GOLogicalColour colour;
  if (parseColor(colour, value))
    return colour;

  wxString error;
  error.Printf(
    _("Invalid color at section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  throw error;
}

int GOConfigReader::ReadInteger(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  int nmin,
  int nmax,
  bool required) {
  return ReadInteger(type, group, key, nmin, nmax, required, nmin);
}

int GOConfigReader::ReadInteger(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  int nmin,
  int nmax,
  bool required,
  int defaultValue) {
  wxString value;
  if (!Read(type, group, key, required, value))
    return defaultValue;

  long retval;
  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  if (!parseLong(retval, value)) {
    if (
      value.Length() && !::wxIsdigit(value[0]) && value[0] != wxT('+')
      && value[0] != wxT('-') && value.CmpNoCase(wxT("none"))
      && !value.IsEmpty()) {
      wxString error;
      error.Printf(
        _("Invalid integer value at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
      throw error;
    }

    retval = wxAtoi(value);
    wxLogWarning(
      _("Invalid integer value at section '%s' entry '%s': %s"),
      group.c_str(),
      key.c_str(),
      value.c_str());
  }

  if (retval < nmin || retval > nmax) {
    if (type == ODFSetting)
      throw wxString::Format(
        _("Out of range value at section '%s' entry '%s': %ld"),
        group,
        key,
        retval);
    wxLogError(
      _("Out of range value at section '%s' entry '%s': %ld. Assumed %d"),
      group,
      key,
      retval,
      defaultValue);
    retval = defaultValue;
  }
  return retval;
}

int GOConfigReader::ReadLong(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  int nmin,
  int nmax,
  bool required) {
  return ReadInteger(type, group, key, nmin, nmax, required);
}

int GOConfigReader::ReadLong(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  int nmin,
  int nmax,
  bool required,
  int defaultValue) {
  return ReadInteger(type, group, key, nmin, nmax, required, defaultValue);
}

double GOConfigReader::ReadFloat(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  double nmin,
  double nmax,
  bool required) {
  return ReadFloat(type, group, key, nmin, nmax, required, nmin);
}

double GOConfigReader::ReadFloat(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  double nmin,
  double nmax,
  bool required,
  double defaultValue) {
  wxString value;
  if (!Read(type, group, key, required, value))
    return defaultValue;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  double retval;
  int pos = value.find(wxT(","), 0);
  if (pos >= 0) {
    wxLogWarning(
      _("Number %s contains locale dependent floating point"), value.c_str());
    value[pos] = wxT('.');
  }
  if (!parseCDouble(retval, value)) {
    wxString error;
    error.Printf(
      _("Invalid float value at section '%s' entry '%s': %s"),
      group.c_str(),
      key.c_str(),
      value.c_str());
    throw error;
  }

  if (retval < nmin || retval > nmax) {
    if (type == ODFSetting)
      throw wxString::Format(
        _("Out of range value at section '%s' entry '%s': %f"),
        group,
        key,
        retval);
    wxLogError(
      _("Out of range value at section '%s' entry '%s': %f. Assumed %f."),
      group,
      key,
      retval,
      defaultValue);
    retval = defaultValue;
  }
  return retval;
}

unsigned GOConfigReader::ReadSize(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  unsigned size_type,
  bool required) {
  return ReadSize(type, group, key, size_type, required, wxT("SMALL"));
}

unsigned GOConfigReader::ReadSize(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  unsigned size_type,
  bool required,
  const wxString &defaultValue) {
  static const int sizes[2][4]
    = {{800, 1007, 1263, 1583}, {500, 663, 855, 1095}};
  wxString value;

  if (!Read(type, group, key, required, value))
    value = defaultValue;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  value.MakeUpper();

  if (value == wxT("SMALL"))
    return sizes[size_type][0];
  else if (value == wxT("MEDIUM"))
    return sizes[size_type][1];
  else if (value == wxT("MEDIUM LARGE"))
    return sizes[size_type][2];
  else if (value == wxT("LARGE"))
    return sizes[size_type][3];

  long size;
  if (parseLong(size, value))
    if (100 <= size && size <= 32000)
      return size;

  wxString error;
  error.Printf(
    _("Invalid size at section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  throw error;
}

unsigned GOConfigReader::ReadFontSize(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required) {
  return ReadFontSize(type, group, key, required, wxT("NORMAL"));
}

unsigned GOConfigReader::ReadFontSize(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  bool required,
  const wxString &defaultValue) {
  wxString value;
  if (!Read(type, group, key, required, value))
    value = defaultValue;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  value.MakeUpper();

  if (value == wxT("SMALL"))
    return 6;
  else if (value == wxT("NORMAL"))
    return 7;
  else if (value == wxT("LARGE"))
    return 10;

  long size;
  if (parseLong(size, value))
    if (1 <= size && size <= 50)
      return size;

  wxString error;
  error.Printf(
    _("Invalid font size at section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  throw error;
}

int GOConfigReader::ReadEnum(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  const struct IniFileEnumEntry *entry,
  unsigned count,
  bool required,
  int defaultValue) {
  int defaultEntry = -1;
  wxString value;
  for (unsigned i = 0; i < count; i++)
    if (entry[i].value == defaultValue)
      defaultEntry = i;
  if (defaultEntry == -1) {
    wxLogError(_("Invalid enum default value"));
    defaultEntry = 0;
  }

  if (!Read(type, group, key, required, value))
    return entry[defaultEntry].value;

  if (value.length() > 0 && value[value.length() - 1] == ' ') {
    if (m_Strict)
      wxLogWarning(
        _("Trailing whitespace at section '%s' entry '%s': %s"),
        group.c_str(),
        key.c_str(),
        value.c_str());
    value.Trim();
  }
  for (unsigned i = 0; i < count; i++)
    if (entry[i].name == value)
      return entry[i].value;

  wxString error;
  error.Printf(
    _("Invalid enum value at section '%s' entry '%s': %s"),
    group.c_str(),
    key.c_str(),
    value.c_str());
  throw error;
}

int GOConfigReader::ReadEnum(
  GOSettingType type,
  const wxString &group,
  const wxString &key,
  const struct IniFileEnumEntry *entry,
  unsigned count,
  bool required) {
  return ReadEnum(type, group, key, entry, count, required, entry[0].value);
}

void GOConfigReader::MarkGroupInUse(const wxString &group) {
  if (m_GroupsInUse.find(group) != m_GroupsInUse.end())
    throw wxString::Format(_("Section %s already in use"), group);
  m_GroupsInUse.insert(group);
}
