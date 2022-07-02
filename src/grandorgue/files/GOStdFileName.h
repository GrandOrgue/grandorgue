/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
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
public:
  static const wxString &getOdfDlgWildcard();
  static const wxString &getPackageDlgWildcard();
};

#endif /* GOSTDFILENAME_H */
