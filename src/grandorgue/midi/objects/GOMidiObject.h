/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECT_H
#define GOMIDIOBJECT_H

#include <vector>

#include <yaml-cpp/yaml.h>

#include "config/GOConfigEnum.h"
#include "midi/dialog-creator/GOMidiConfigDispatcher.h"
#include "sound/GOSoundStateHandler.h"

#include "GOSaveableObject.h"
#include "GOStringSet.h"

class GOMidiElement;
class GOMidiMap;
class GOMidiObjectContext;
class GOMidiReceiver;
class GOMidiShortcutReceiver;
class GOMidiSender;
class GOOrganModel;

class GOMidiObject : public GOSaveableObject, public GOMidiConfigDispatcher {
public:
public:
  static constexpr int ELEMENT_TYPE_NONE = -1;

  enum ObjectType {
    OBJECT_TYPE_LABEL,
    OBJECT_TYPE_RANK,
    OBJECT_TYPE_MANUAL,
    OBJECT_TYPE_ENCLOSURE,
    OBJECT_TYPE_BUTTON,
    OBJECT_TYPE_PISTON,
    OBJECT_TYPE_STOP,
    OBJECT_TYPE_SWITCH,
    OBJECT_TYPE_TREMULANT,
    OBJECT_TYPE_GENERAL,
    OBJECT_TYPE_DIVISIONAL,
    OBJECT_TYPE_COUPLER,
    OBJECT_TYPE_DIVISIONAL_COUPLER,
  };

  static const GOConfigEnum OBJECT_TYPES;
  static const wxString OBJECT_TYPE_NAMES[];

private:
  GOMidiMap &r_MidiMap;
  ObjectType m_ObjectType;

  // a name that may be translated (for built-in objects)
  wxString m_name;
  // a name that is never translated. May be empty which causes that m_name is
  // used
  wxString m_HardName;

  GOMidiSender *p_MidiSender;
  GOMidiReceiver *p_MidiReceiver;
  GOMidiShortcutReceiver *p_ShortcutReceiver;
  GOMidiSender *p_DivisionSender;

  const GOMidiObjectContext *p_context;

protected:
  template <typename MidiElementType>
  static int getElementType(const MidiElementType *pEl) {
    return pEl ? (int)pEl->GetType() : ELEMENT_TYPE_NONE;
  }

  GOMidiObject(GOMidiMap &midiMap, ObjectType objectType);

public:
  ObjectType GetObjectType() const { return m_ObjectType; }
  GOMidiSender *GetMidiSender() const { return p_MidiSender; }
  void SetMidiSender(GOMidiSender *pMidiSender) { p_MidiSender = pMidiSender; }
  GOMidiReceiver *GetMidiReceiver() const { return p_MidiReceiver; }
  void SetMidiReceiver(GOMidiReceiver *pMidiReceiver) {
    p_MidiReceiver = pMidiReceiver;
  }
  GOMidiShortcutReceiver *GetMidiShortcutReceiver() const {
    return p_ShortcutReceiver;
  }
  void SetMidiShortcutReceiver(GOMidiShortcutReceiver *pShortcutReceiver) {
    p_ShortcutReceiver = pShortcutReceiver;
  }
  GOMidiSender *GetDivisionSender() const { return p_DivisionSender; }
  void SetDivisionSender(GOMidiSender *pDivisionSender) {
    p_DivisionSender = pDivisionSender;
  }

  int GetSenderType() const;
  int GetReceiverType() const;
  int GetShortcutReceiverType() const;
  int GetDivisionSenderType() const;

  virtual void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap);
  virtual void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const;
  virtual void AfterMidiLoaded() {}

private:
  void InitMidiObject(
    GOConfigReader &cfg, const wxString &group, const wxString &name);

  void SubToYaml(
    YAML::Node &yamlNode,
    const wxString &objPath,
    const GOMidiElement *pEl) const;
  void SubFromYaml(
    const YAML::Node &objNode,
    const wxString &objPath,
    const wxString &subName,
    GOMidiElement *pEl,
    GOStringSet &usedPaths);

public:
  GOMidiMap &GetMidiMap() { return r_MidiMap; }

  const wxString &GetMidiTypeCode() const {
    return OBJECT_TYPES.GetName((int)m_ObjectType);
  }

  const wxString &GetMidiTypeName() const {
    return OBJECT_TYPE_NAMES[m_ObjectType];
  }

  const wxString &GetName() const { return m_name; }
  void SetName(const wxString &name) { m_name = name; }

  const GOMidiObjectContext *GetContext() const { return p_context; }
  void SetContext(const GOMidiObjectContext *pContext) { p_context = pContext; }

  const wxString &GetHardName() { return m_HardName; }
  void SetHardName(const wxString &name) { m_HardName = name; }
  wxString GetNameForContext() const {
    return m_HardName.IsEmpty() ? wxString(m_name).Trim(true).Trim(false)
                                : m_HardName;
  }

  wxString GetPath() const;

  wxString GetContextTitle() const;

  bool IsMidiConfigured() const;

  void CopyMidiSettingFrom(const GOMidiObject &objFrom);

  virtual void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) {
    InitMidiObject(cfg, group, name);
  }

  void LoadMidiSettings(GOConfigReader &cfg) {
    LoadMidiObject(cfg, m_group, r_MidiMap);
  }

  void SaveMidiSettings(GOConfigWriter &cfg) {
    SaveMidiObject(cfg, m_group, r_MidiMap);
  }

  virtual void Load(
    GOConfigReader &cfg, const wxString &group, const wxString &name) {
    InitMidiObject(cfg, group, name);
  }

  virtual void Save(GOConfigWriter &cfg) override {
    SaveMidiObject(cfg, m_group, r_MidiMap);
  }

  void ToYaml(YAML::Node &yamlNode) const;
  void FromYaml(const YAML::Node &objNode, const wxString &objPath);

  virtual bool IsReadOnly() const { return false; }
};

#endif
