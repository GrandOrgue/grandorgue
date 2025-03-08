/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOYamlInModel.h"

#include <wx/intl.h>
#include <wx/wfstream.h>

#include "GOSaveableToYaml.h"

static std::vector<char> load_file_bytes(const wxString &filePath) {
  wxFile file;
  if (!file.Open(filePath)) {
    throw wxString::Format(
      _("Failed to open '%s': %s"), filePath, strerror(file.GetLastError()));
  }
  std::vector<char> content;
  content.reserve(file.Length());
  char buf[8 * 1024]; // 8 KiB
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buf, sizeof(buf))) != 0) {
    if (bytesRead == wxInvalidOffset) {
      throw wxString::Format(
        _("Failed to read '%s': %s"), filePath, strerror(file.GetLastError()));
    }
    content.insert(content.end(), &buf[0], &buf[bytesRead]);
  }
  return content;
}

static wxString load_file_text_with_encoding_detection(
  const wxString &filePath) {
  std::vector<char> content = load_file_bytes(filePath);
  wxBOM detectedBOM = wxConvAuto::DetectBOM(&content[0], content.size());
  if (detectedBOM != wxBOM_None && detectedBOM != wxBOM_Unknown) {
    // We know what encoding was used for that file.
    // wxConvAuto will use BOM to determine encoding and to decode file content.
    // Note: newer GO versions export yaml files with UTF-8-BOM.
    return wxString(&content[0], wxConvAuto(), content.size());
  } else {
    // Use encoding that was used in older GO versions (system default)
    return wxString(&content[0], *wxConvCurrent, content.size());
  }
}

static YAML::Node load_yaml_from_file(const wxString &fileName) {
  wxString fileContent = load_file_text_with_encoding_detection(fileName);
  // Note: wxScopedCharBuffer may point to internals of wxString above
  // fileContent must not be destructed while fileContentInUtf8 is in use
  wxScopedCharBuffer fileContentInUtf8 = fileContent.utf8_str();

  return YAML::Load(fileContentInUtf8.data());
}

GOYamlInModel::GOYamlInModel(
  const wxString &organName,
  const wxString &fileName,
  const wxString &contentType)
  : m_GlobalNode(load_yaml_from_file(fileName)) {
  const wxString fileContentType = getFileContentType(m_GlobalNode);

  if (fileContentType != contentType)
    throw wxString::Format(
      _("The file '%s' is a %s file instead of a %s file"),
      fileName,
      fileContentType,
      contentType);
}

const GOYamlInModel &GOYamlInModel::operator>>(
  GOSaveableToYaml &saveableObj) const {
  m_GlobalNode >> saveableObj;
  return *this;
}
