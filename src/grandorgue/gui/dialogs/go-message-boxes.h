/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_MESSAGE_BOXES_H
#define GO_MESSAGE_BOXES_H

class wxString;

/**
 * Check the organName of the imported file. If it differs from the current
 * organ name then ask the user
 * @param thisOrganName current organ name
 * @param contentType the content type of the file
 * @param fileName the file name to import
 * @param fileOrganName the organ the file was saved of
 * @return true if the churchNames are the same or the user agree with importing
 *   the combination file
 */
extern bool is_to_import_to_this_organ(
  const wxString &thisOrganName,
  const wxString &contentType,
  const wxString &fileName,
  const wxString &fileOrganName);

#endif /* GO_MESSAGE_BOXES_H */
