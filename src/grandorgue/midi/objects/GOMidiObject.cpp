/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include <wx/intl.h>

#include "midi/elements/GOMidiReceiver.h"
#include "midi/elements/GOMidiSender.h"
#include "midi/elements/GOMidiShortcutReceiver.h"
#include "model/GOOrganModel.h"
#include "yaml/go-wx-yaml.h"

#include "GOMidiObjectContext.h"

#include "GOMidiObjectContext.h"

GOMidiObject::GOMidiObject(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiType)
  : r_OrganModel(organModel),
    r_MidiMap(organModel.GetConfig().GetMidiMap()),
    r_MidiTypeCode(midiTypeCode),
    r_MidiTypeName(midiType),
    p_MidiSender(nullptr),
    p_MidiReceiver(nullptr),
    p_ShortcutReceiver(nullptr),
    p_DivisionSender(nullptr),
    p_context(nullptr) {
  r_OrganModel.RegisterSoundStateHandler(this);
  r_OrganModel.RegisterMidiObject(this);
}

GOMidiObject::~GOMidiObject() {
  r_OrganModel.UnregisterSaveableObject(this);
  r_OrganModel.UnRegisterMidiObject(this);
  r_OrganModel.UnRegisterSoundStateHandler(this);
}

static const wxString WX_CONTEXT_SEPARATOR = wxT(".");

wxString GOMidiObject::GetPath() const {
  std::vector<wxString> contextNames = GOMidiObjectContext::getNames(p_context);

  contextNames.push_back(GetNameForContext());

  wxString path;

  for (const wxString &name : contextNames) {
    if (!path.IsEmpty())
      path = path + WX_CONTEXT_SEPARATOR;
    path = path + name;
  }
  return path;
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
  r_OrganModel.RegisterSaveableObject(this);
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

void GOMidiObject::ToYaml(YAML::Node &yamlNode) const {
  const wxString &name = GetName();
  const wxString &nameForContext = GetNameForContext();
  YAML::Node objNode;

  SubToYaml(objNode, WX_RECEIVE, p_MidiReceiver);
  SubToYaml(objNode, WX_SEND, p_MidiSender);
  SubToYaml(objNode, WX_SHORTCUT, p_ShortcutReceiver);
  SubToYaml(objNode, WX_DIVISION, p_DivisionSender);
  if (!objNode.IsNull() && nameForContext != name) {
    YAML::Node newNode;

    // put name at tfirst, then put all other keys
    newNode[WX_NAME] = name;
    for (const auto &e : objNode)
      newNode[e.first] = e.second;
    objNode = newNode;
  }
  put_to_map_by_path_if_not_null(
    yamlNode,
    GOMidiObjectContext::getNames(p_context),
    GetNameForContext(),
    objNode);
}

void GOMidiObject::FromYaml(
  const YAML::Node &yamlNode, GOStringSet &usedPaths) {
  const std::vector<wxString> parentNames
    = GOMidiObjectContext::getNames(p_context);
  const wxString &name = GetName();
  const wxString &nameForContext = GetNameForContext();
  YAML::Node objNode
    = get_from_map_by_path_or_null(yamlNode, parentNames, nameForContext);
  wxString objPath;

  for (const auto &parentName : parentNames)
    objPath = get_child_path(objPath, parentName);
  objPath = get_child_path(objPath, nameForContext);

  if (nameForContext != name)
    wxString storedName
      = read_string(objNode, objPath, WX_NAME, false, usedPaths);
  SubFromYaml(objNode, objPath, WX_RECEIVE, p_MidiReceiver, usedPaths);
  SubFromYaml(objNode, objPath, WX_SEND, p_MidiSender, usedPaths);
  SubFromYaml(objNode, objPath, WX_SHORTCUT, p_ShortcutReceiver, usedPaths);
  SubFromYaml(objNode, objPath, WX_DIVISION, p_DivisionSender, usedPaths);
}

void GOMidiObject::ShowConfigDialog() {
  const bool isReadOnly = IsReadOnly();
  const wxString title
    = wxString::Format(_("MIDI-Settings for %s - %s"), r_MidiTypeName, m_name);
  const wxString selector
    = wxString::Format(wxT("%s.%s"), r_MidiTypeCode, m_name);

  r_OrganModel.ShowMIDIEventDialog(
    this,
    title,
    selector,
    isReadOnly ? nullptr : p_MidiReceiver,
    p_MidiSender,
    isReadOnly ? nullptr : p_ShortcutReceiver,
    p_DivisionSender,
    this);
}
