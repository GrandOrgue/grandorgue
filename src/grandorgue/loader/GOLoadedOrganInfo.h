/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADEDORGANINFO_H
#define GOLOADEDORGANINFO_H

#include <wx/string.h>

/** Everything GOOrganReader discovers about an organ that has no
 * equivalent field in GOOrgan and must outlive a single Load() call. */
struct GOLoadedOrganInfo {
  /** Path of the per-user CMB this organ is saved to. */
  wxString settingsFilePath;
  /** Path of the precomputed sample cache file for this organ. */
  wxString cacheFilePath;
  /** Hash of the ODF's contents, written into exported CMB files. */
  wxString odfHash;
  /** True if a CMB (explicit, saved, bundled, or archived) was applied. */
  bool isCustomized = false;
};

#endif /* GOLOADEDORGANINFO_H */
