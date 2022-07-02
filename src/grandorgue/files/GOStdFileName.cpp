#include "GOStdFileName.h"

#include <wx/translation.h>

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
