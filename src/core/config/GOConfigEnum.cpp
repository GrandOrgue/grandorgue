/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOConfigEnum.h"

#include <algorithm>

static const wxString WX_EMPTY;

const wxString &GOConfigEnum::GetName(int value) const {
  const auto end = m_entries.end();
  const auto found
    = std::find_if(m_entries.cbegin(), end, [value](const Entry &e) {
        return e.value == value;
      });

  return found != end ? found->name : WX_EMPTY;
}

int GOConfigEnum::GetValue(const wxString &name, int defaultValue) const {
  const auto end = m_entries.end();
  const auto found = std::find_if(
    m_entries.cbegin(), end, [name](const Entry &e) { return e.name == name; });

  return found != end ? found->value : defaultValue;
}
