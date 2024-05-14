/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTDFILENAME_H
#define GOSTDFILENAME_H

#include <wx/string.h>

/**
 * Definies the standard file names and patterns
 */
class GOStdFileName {
private:
  static const wxString universal_wildcard;

public:
  static const wxString SETTING_FILE_EXT;
  static const wxString CACHE_FILE_EXT;
  static const wxString INDEX_FILE_EXT;

private:
  static wxString composeOrganFileName(
    const wxString &organHash,
    const wxString presetStr,
    const wxString &extension);
  static wxString composeOrganFileName(
    const wxString &organHash,
    const unsigned presetNum,
    const wxString &extension);
  static wxString composeOrganFilePattern(
    const wxString &organHash, const wxString &extension) {
    return composeOrganFileName(organHash, universal_wildcard, extension);
  }

public:
  static const wxString &getOdfDlgWildcard();
  static const wxString &getPackageDlgWildcard();

  static wxString extractOrganHash(const wxString &fullFileName);
  static wxString extractIndexHash(const wxString &fullFileName);
  static wxString composeCacheFilePattern(const wxString &organHash) {
    return composeOrganFilePattern(organHash, CACHE_FILE_EXT);
  }
  static wxString composeCacheFilePattern() {
    return composeCacheFilePattern(universal_wildcard);
  }
  static wxString composeCacheFileName(
    const wxString &organHash, const unsigned presetNum) {
    return composeOrganFileName(organHash, presetNum, CACHE_FILE_EXT);
  }
  static wxString composeIndexFilePattern() {
    return composeOrganFileName(
      universal_wildcard, wxEmptyString, INDEX_FILE_EXT);
  }
  static wxString composeIndexFileName(const wxString &packageHash) {
    return composeOrganFileName(packageHash, wxEmptyString, INDEX_FILE_EXT);
  }
  static wxString composeSettingFilePattern(const wxString &organHash) {
    return composeOrganFilePattern(organHash, SETTING_FILE_EXT);
  }
  static wxString composeSettingFilePattern() {
    return composeSettingFilePattern(universal_wildcard);
  }
  static wxString composeSettingFileName(
    const wxString &organHash, const unsigned presetNum) {
    return composeOrganFileName(organHash, presetNum, SETTING_FILE_EXT);
  }

  static wxString composeFullPath(
    const wxString &dirPath, const wxString &fileName);
};

#endif /* GOSTDFILENAME_H */
