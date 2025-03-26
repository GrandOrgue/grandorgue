/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGENUM_H
#define GOCONFIGENUM_H

#include <vector>

#include <wx/string.h>

class GOConfigEnum {
public:
  struct Entry {
    wxString name;
    int value;
  };

private:
  const std::vector<Entry> m_entries;

public:
  GOConfigEnum(const std::vector<Entry> &entries) : m_entries(entries) {}

  const wxString &GetName(int value) const;
  int GetValue(const wxString &name, int defaultValue) const;
  int GetFirstValue() const { return m_entries[0].value; }
};

#endif /* GOCONFIGENUM_H */
