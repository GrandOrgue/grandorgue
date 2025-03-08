/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOYamlModelBase.h"

#include <wx/datetime.h>
#include <yaml-cpp/yaml.h>

#include "go-wx-yaml.h"
#include "go_defs.h"

const char *const INFO = "info";
const char *const CONTENT_TYPE = "content-type";
const char *const ORGAN_NAME = "organ-name";
const char *const GRANDORGUE_VERSION = "grandorgue-version";
const char *const SAVED_TIME = "saved_time";

void GOYamlModelBase::putInfoNode(
  const wxString &contentType,
  const wxString &organName,
  YAML::Node &globalNode) {
  YAML::Node infoNode = globalNode[INFO];

  infoNode[CONTENT_TYPE] = contentType;
  infoNode[ORGAN_NAME] = organName;
  infoNode[GRANDORGUE_VERSION] = APP_VERSION;
  infoNode[SAVED_TIME] = wxDateTime::Now().Format();
}

static wxString get_info_field(
  const YAML::Node &globalNode, const wxString &fieldName) {
  return globalNode[INFO][fieldName].as<wxString>();
}

wxString GOYamlModelBase::getFileContentType(const YAML::Node &globalNode) {
  return get_info_field(globalNode, CONTENT_TYPE);
}

wxString GOYamlModelBase::geFileOrganName(const YAML::Node &globalNode) {
  return get_info_field(globalNode, ORGAN_NAME);
}
