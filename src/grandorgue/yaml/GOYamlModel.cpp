/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOYamlModel.h"

#include <wx/datetime.h>
#include <wx/intl.h>
#include <wx/wfstream.h>
#include <yaml-cpp/yaml.h>

#include "GOSaveableToYaml.h"
#include "go-wx-yaml.h"
#include "go_defs.h"

static const uint8_t UTF8_BOM[] = {0xEF, 0xBB, 0xBF};

const wxString WX_INFO = "info";
const wxString WX_CONTENT_TYPE = "content-type";
const wxString WX_ORGAN_NAME = "organ-name";
const wxString WX_GRANDORGUE_VERSION = "grandorgue-version";
const wxString WX_SAVED_TIME = "saved_time";

static wxString get_info_field(
  const YAML::Node &globalNode,
  const wxString &fieldName,
  bool isRequired,
  GOStringSet &usedPaths) {
  return read_string(
    globalNode[WX_INFO], WX_INFO, fieldName, isRequired, usedPaths);
}

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

GOYamlModel::In::In(
  const wxString &organName,
  const wxString &fileName,
  const wxString &contentType)
  : m_GlobalNode(load_yaml_from_file(fileName)) {
  const wxString fileContentType
    = get_info_field(m_GlobalNode, WX_CONTENT_TYPE, true, m_UsedPaths);

  if (fileContentType != contentType)
    throw wxString::Format(
      _("The file '%s' is a %s file instead of a %s file"),
      fileName,
      fileContentType,
      contentType);
  m_OrganName = get_info_field(m_GlobalNode, WX_ORGAN_NAME, false, m_UsedPaths);
  get_info_field(m_GlobalNode, WX_GRANDORGUE_VERSION, false, m_UsedPaths);
  get_info_field(m_GlobalNode, WX_SAVED_TIME, false, m_UsedPaths);
}

const GOYamlModel::In &GOYamlModel::In::operator>>(
  GOSaveableToYaml &saveableObj) const {
  m_GlobalNode >> saveableObj;
  return *this;
}

void GOYamlModel::In::CheckAllUsed() const {
  check_all_used(m_GlobalNode, wxEmptyString, m_UsedPaths);
}

GOYamlModel::Out::Out(const wxString &organName, const wxString &contentType)
  : m_GlobalNode(YAML::NodeType::Map) {
  YAML::Node infoNode = m_GlobalNode[WX_INFO];

  infoNode[WX_CONTENT_TYPE] = contentType;
  infoNode[WX_ORGAN_NAME] = organName;
  infoNode[WX_GRANDORGUE_VERSION] = APP_VERSION;
  infoNode[WX_SAVED_TIME] = wxDateTime::Now().Format();
}

GOYamlModel::Out &GOYamlModel::Out::operator<<(
  const GOSaveableToYaml &saveableObj) {
  m_GlobalNode << saveableObj;
  return *this;
}

wxString GOYamlModel::Out::writeTo(const wxString &fileName) {
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
