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

static const wxString WX_DIVISIONAL_GROUP_SUFFIX = wxT("Divisional");

const GOConfigEnum GOMidiObject::OBJECT_TYPES({
  {wxT("Label"), (int)OBJECT_TYPE_LABEL},
  {wxT("Rank"), (int)OBJECT_TYPE_RANK},
  {wxT("Manual"), (int)OBJECT_TYPE_MANUAL},
  {wxT("Enclosure"), (int)OBJECT_TYPE_ENCLOSURE},
  {wxT("Button"), (int)OBJECT_TYPE_BUTTON},
  {wxT("Piston"), (int)OBJECT_TYPE_PISTON},
  {wxT("Stop"), (int)OBJECT_TYPE_STOP},
  {wxT("Switch"), (int)OBJECT_TYPE_SWITCH},
  {wxT("Tremulant"), (int)OBJECT_TYPE_TREMULANT},
  {wxT("General"), (int)OBJECT_TYPE_GENERAL},
  {wxT("Divisional"), (int)OBJECT_TYPE_DIVISIONAL},
  {wxT("Coupler"), (int)OBJECT_TYPE_COUPLER},
  {wxT("DivisionalCoupler"), (int)OBJECT_TYPE_DIVISIONAL_COUPLER},
});

const wxString GOMidiObject::OBJECT_TYPE_NAMES[] = {
  _("Label"),
  _("Rank"),
  _("Manual"),
  _("Enclosure"),
  _("Button"),
  _("Piston"),
  _("Stop"),
  _("Switch"),
  _("Tremulant"),
  _("General"),
  _("Divisional"),
  _("Coupler"),
  _("Divisional Coupler"),
};

GOMidiObject::GOMidiObject(GOMidiMap &midiMap, ObjectType objectType)
  : r_MidiMap(midiMap),
    m_ObjectType(objectType),
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

static wxString divisional_group(const wxString &group) {
  return group + WX_DIVISIONAL_GROUP_SUFFIX;
}

int GOMidiObject::GetSenderType() const { return getElementType(p_MidiSender); }
int GOMidiObject::GetReceiverType() const {
  return getElementType(p_MidiReceiver);
}
int GOMidiObject::GetShortcutReceiverType() const {
  return getElementType(p_ShortcutReceiver);
}
int GOMidiObject::GetDivisionSenderType() const {
  return getElementType(p_DivisionSender);
}

void GOMidiObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  if (p_MidiSender)
    p_MidiSender->Load(cfg, group, midiMap);
  if (!IsReadOnly()) {
    if (p_MidiReceiver)
      p_MidiReceiver->Load(false, cfg, group, midiMap);
    if (p_ShortcutReceiver)
      p_ShortcutReceiver->Load(cfg, group);
  }
  if (p_DivisionSender)
    p_DivisionSender->Load(cfg, divisional_group(group), midiMap);
}

void GOMidiObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const {
  if (p_MidiSender)
    p_MidiSender->Save(cfg, group, midiMap);
  if (!IsReadOnly()) {
    if (p_MidiReceiver)
      p_MidiReceiver->Save(cfg, group, midiMap);
    if (p_ShortcutReceiver)
      p_ShortcutReceiver->Save(cfg, group);
  }
  if (p_DivisionSender)
    p_DivisionSender->Save(cfg, divisional_group(group), midiMap);
}

bool GOMidiObject::IsMidiConfigured() const {
  return (p_MidiSender && p_MidiSender->IsMidiConfigured())
    || (p_MidiReceiver && p_MidiReceiver->IsMidiConfigured())
    || (p_ShortcutReceiver && p_ShortcutReceiver->IsMidiConfigured())
    || (p_DivisionSender && p_DivisionSender->IsMidiConfigured());
}

void GOMidiObject::CopyMidiSettingFrom(const GOMidiObject &objFrom) {
  if (p_MidiSender && objFrom.p_MidiSender)
    p_MidiSender->RenewFrom(*objFrom.p_MidiSender);
  if (!IsReadOnly()) {
    if (p_MidiReceiver && objFrom.p_MidiReceiver)
      p_MidiReceiver->RenewFrom(*objFrom.p_MidiReceiver);
    if (p_ShortcutReceiver && objFrom.p_ShortcutReceiver)
      p_ShortcutReceiver->RenewFrom(*objFrom.p_ShortcutReceiver);
  }
  if (p_DivisionSender && objFrom.p_DivisionSender)
    p_DivisionSender->RenewFrom(*objFrom.p_DivisionSender);
}

void GOMidiObject::InitMidiObject(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  SetGroup(group);
  m_name = name;
  LoadMidiObject(cfg, group, r_MidiMap);
  AfterMidiLoaded();
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
  AfterMidiLoaded();
}
