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

std::vector<wxString> GOMidiObjectContext::getNames(
  const GOMidiObjectContext *pContext) {
  std::vector<wxString> res;

  for (; pContext; pContext = pContext->p_parent)
    res.insert(res.begin(), pContext->m_name);
  return res;
}

wxString GOMidiObjectContext::getPath(
  const GOMidiObjectContext *pContext, const wxString &name) {
  std::vector<wxString> contextNames = getNames(pContext);

  contextNames.push_back(name);

  wxString path;

  for (const wxString &name : contextNames) {
    if (!path.IsEmpty())
      path = path + WX_PATH_SEPARATOR;
    path = path + name;
  }
  return path;
}

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
