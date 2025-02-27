/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObjectContext.h"

const wxString WX_PATH_SEPARATOR = "/";

GOMidiObjectContext::GOMidiObjectContext(
  const wxString &name,
  const wxString &title,
  const GOMidiObjectContext *pParent)
  : m_name(name), m_title(title), p_parent(pParent) {}

wxString GOMidiObjectContext::getFullTitle(
  const GOMidiObjectContext *pContext) {
  wxString path;

  for (; pContext; pContext = pContext->p_parent) {
    if (!path.IsEmpty())
      path = WX_PATH_SEPARATOR + path;
    path = pContext->m_title + path;
  }
  return path;
}
