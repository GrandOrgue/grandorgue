/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include "midi/elements/GOMidiReceiver.h"
#include "midi/elements/GOMidiSender.h"
#include "midi/elements/GOMidiShortcutReceiver.h"
#include "model/GOOrganModel.h"
#include "yaml/go-wx-yaml.h"

#include "GOMidiObjectContext.h"

GOMidiObject::GOMidiObject(
  GOMidiMap &midiMap, const wxString &midiTypeCode, const wxString &midiType)
  : r_MidiMap(midiMap),
    r_MidiTypeCode(midiTypeCode),
    r_MidiTypeName(midiType),
    p_MidiSender(nullptr),
    p_MidiReceiver(nullptr),
    p_ShortcutReceiver(nullptr),
    p_DivisionSender(nullptr),
    p_context(nullptr) {}

wxString GOMidiObject::GetPath() const {
  return GOMidiObjectContext::getPath(p_context, GetNameForContext());
}

wxString GOMidiObject::GetContextTitle() const {
  return GOMidiObjectContext::getFullTitle(p_context);
}

bool GOMidiObject::IsMidiConfigured() const {
  return (p_MidiSender && p_MidiSender->IsMidiConfigured())
    || (p_MidiReceiver && p_MidiReceiver->IsMidiConfigured())
    || (p_ShortcutReceiver && p_ShortcutReceiver->IsMidiConfigured())
    || (p_DivisionSender && p_DivisionSender->IsMidiConfigured());
}

void GOMidiObject::InitMidiObject(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  SetGroup(group);
  m_name = name;
  LoadMidiObject(cfg, group, r_MidiMap);
}

void GOMidiObject::SubToYaml(
  YAML::Node &yamlNode,
  const wxString &subName,
  const GOMidiElement *pEl) const {
  if (pEl) {
    YAML::Node subNode;

    pEl->ToYaml(subNode, r_MidiMap);
    put_to_map_if_not_null(yamlNode, subName, subNode);
  }
}

void GOMidiObject::SubFromYaml(
  const YAML::Node &objNode,
  const wxString &objPath,
  const wxString &subName,
  GOMidiElement *pEl,
  GOStringSet &usedPaths) {
  if (pEl) {
    YAML::Node subNode = get_from_map_or_null(objNode, subName);

    pEl->FromYaml(
      subNode, get_child_path(objPath, subName), r_MidiMap, usedPaths);
  }
}

static const wxString WX_RECEIVE = "receive";
static const wxString WX_SEND = "send";
static const wxString WX_SHORTCUT = "shortcut";
static const wxString WX_DIVISION = "division";
static const wxString WX_NAME = "name";
static const wxString WX_PATH = "path";

void GOMidiObject::ToYaml(YAML::Node &yamlNode) const {
  const wxString &name = GetName();
  const wxString &nameForContext = GetNameForContext();
  YAML::Node objNode;

  SubToYaml(objNode, WX_RECEIVE, p_MidiReceiver);
  SubToYaml(objNode, WX_SEND, p_MidiSender);
  SubToYaml(objNode, WX_SHORTCUT, p_ShortcutReceiver);
  SubToYaml(objNode, WX_DIVISION, p_DivisionSender);
  if (!objNode.IsNull()) {
    // put path and name at first, then put all other keys
    yamlNode[WX_PATH] = GetPath();
    if (nameForContext != name)
      yamlNode[WX_NAME] = name;
    for (const auto &e : objNode)
      yamlNode[e.first] = e.second;
  }
}

void GOMidiObject::FromYaml(
  const YAML::Node &objNode, const wxString &objPath) {
  const wxString &name = GetName();
  const wxString &nameForContext = GetNameForContext();
  GOStringSet usedPaths;

  read_string(objNode, objPath, WX_PATH, false, usedPaths);
  if (nameForContext != name)
    wxString storedName
      = read_string(objNode, objPath, WX_NAME, false, usedPaths);
  SubFromYaml(objNode, objPath, WX_RECEIVE, p_MidiReceiver, usedPaths);
  SubFromYaml(objNode, objPath, WX_SEND, p_MidiSender, usedPaths);
  SubFromYaml(objNode, objPath, WX_SHORTCUT, p_ShortcutReceiver, usedPaths);
  SubFromYaml(objNode, objPath, WX_DIVISION, p_DivisionSender, usedPaths);
  check_all_used(objNode, objPath, usedPaths);
}
