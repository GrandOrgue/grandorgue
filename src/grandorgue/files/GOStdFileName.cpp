#include "GOStdFileName.h"

#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/translation.h>

const wxString GOStdFileName::universal_wildcard = wxT("*");
const wxString GOStdFileName::SETTING_FILE_EXT = wxT("cmb");
const wxString GOStdFileName::CACHE_FILE_EXT = wxT("cache");

static wxString odf_dlg_wildcard;
static wxString package_dlg_wildcard;

const wxString &GOStdFileName::getOdfDlgWildcard() {
  if (odf_dlg_wildcard.IsEmpty())
    odf_dlg_wildcard = _("Sample set definition files (*.organ)") + "|*.organ";
  return odf_dlg_wildcard;
}

const wxString &GOStdFileName::getPackageDlgWildcard() {
  if (package_dlg_wildcard.IsEmpty())
    package_dlg_wildcard = _("Organ package (*.orgue)") + "|*.orgue";
  return package_dlg_wildcard;
}

wxString GOStdFileName::composeOrganFileName(
  const wxString &organHash,
  const wxString presetStr,
  const wxString &extension) {
  return wxString::Format("%s-%s.%s", organHash, presetStr, extension);
}

wxString GOStdFileName::composeOrganFileName(
  const wxString &organHash,
  const unsigned presetNum,
  const wxString &extension) {
  return composeOrganFileName(
    organHash, wxString::Format(wxT("%u"), presetNum), extension);
}

wxString GOStdFileName::extractOrganHash(const wxString &fullFileName) {
  const wxString fileName = wxFileName(fullFileName).GetFullName();
  wxRegEx regEx("([[:xdigit:]]+)-([[:digit:]]+)\\.([[:alpha:]]+)");
  wxString res;

  if (regEx.Matches(fileName))
    res = regEx.GetMatch(fileName, 1);
  return res;
}
