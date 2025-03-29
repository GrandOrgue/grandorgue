/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "go-message-boxes.h"

#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

bool is_to_import_to_this_organ(
  const wxString &thisOrganName,
  const wxString &contentType,
  const wxString &fileName,
  const wxString &fileOrganName) {
  bool isToImport = true;

  if (fileOrganName != thisOrganName) {
    wxLogWarning(
      _("This %s file '%s' was originally made for another organ '%s'"),
      contentType,
      fileName,
      fileOrganName);
    isToImport = wxMessageBox(
                   wxString::Format(
                     _("This %s file '%s' was originally made for "
                       "another organ '%s'. Importing it can cause various "
                       "problems. Should it really be imported?"),
                     contentType,
                     fileName,
                     fileOrganName),
                   _("Import File"),
                   wxYES_NO,
                   NULL)
      == wxYES;
  }
  return isToImport;
}
