/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOYamlOutModel.h"

#include <wx/wfstream.h>

#include "GOSaveableToYaml.h"

static const uint8_t UTF8_BOM[] = {0xEF, 0xBB, 0xBF};

GOYamlOutModel::GOYamlOutModel(
  const wxString &organName, const wxString &contentType)
  : m_GlobalNode(YAML::NodeType::Map) {
  putInfoNode(contentType, organName, m_GlobalNode);
}

GOYamlOutModel &GOYamlOutModel::operator<<(
  const GOSaveableToYaml &saveableObj) {
  m_GlobalNode << saveableObj;
  return *this;
}

wxString GOYamlOutModel::writeTo(const wxString &fileName) {
  wxString errMsg;
  wxFileOutputStream fOS(fileName);

  if (fOS.IsOk()) {
    YAML::Emitter outYaml;

    outYaml << YAML::BeginDoc << m_GlobalNode;

    if (
      !fOS.WriteAll(UTF8_BOM, sizeof(UTF8_BOM))
      || !fOS.WriteAll(outYaml.c_str(), outYaml.size()))
      errMsg.Printf(
        wxT("Unable to write all the data to the file '%s'"), fileName);
    fOS.Close();
  } else
    errMsg.Printf(wxT("Unable to open the file '%s' for writing"), fileName);
  return errMsg;
}
