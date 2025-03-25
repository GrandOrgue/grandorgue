/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECTCONTEXT_H
#define GOMIDIOBJECTCONTEXT_H

#include <wx/string.h>

/**
 * This class is used for describing a place where a midi object is used.
 * There is a hierarhial tree of contexts. For example, setter, manuals/1/stops,
 * manuals/1/divisionals
 */

class GOMidiObjectContext {
private:
  // Name. Not translated. Used for exports
  wxString m_name;

  // Title. Translated. Used in UI
  wxString m_title;

  // Parent context
  const GOMidiObjectContext *p_parent;

public:
  GOMidiObjectContext(
    const wxString &name,
    const wxString &title,
    const GOMidiObjectContext *pParent = nullptr);
  GOMidiObjectContext()
    : GOMidiObjectContext(wxEmptyString, wxEmptyString, nullptr) {}

  const wxString &GetName() const { return m_name; }
  const wxString &GetTitle() const { return m_title; }
  void SetTitle(const wxString title) { m_title = title; }
  const GOMidiObjectContext *GetParent() const { return p_parent; }

  static wxString getFullTitle(const GOMidiObjectContext *pContext);
};

#endif /* GOMIDIOBJECTCONTEXT_H */
