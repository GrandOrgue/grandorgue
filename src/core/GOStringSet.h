/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTRINGSET_H
#define GOSTRINGSET_H

#include <unordered_set>

#include <wx/hashmap.h>
#include <wx/string.h>

using GOStringSet = std::unordered_set<wxString, wxStringHash, wxStringEqual>;

#endif /* GOSTRINGSET_H */
