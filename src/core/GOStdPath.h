/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTDPATH_H
#define GOSTDPATH_H

#include <wx/string.h>

class GOStdPath {
public:
  static void InitLocaleDir();
  static wxString GetBaseDir();
  static wxString GetResourceDir();
  static wxString GetConfigDir();
  static wxString GetDocumentDir();
  static wxString GetGrandOrgueDir();
  static wxString GetGrandOrgueSubDir(const wxString &subdir);
};

#endif
