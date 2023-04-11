/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSetter.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>
#include <yaml-cpp/yaml.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "control/GOCallbackButtonControl.h"
#include "control/GOGeneralButtonControl.h"
#include "yaml/go-wx-yaml.h"

#include "GOEvent.h"
#include "GOOrganController.h"
#include "go_ids.h"

#define FRAME_GENERALS 1000
#define GENERAL_BANKS 20
#define GENERALS 50
#define CRESCENDO_STEPS 32

enum {
  ID_SETTER_PREV = 0,
  ID_SETTER_NEXT,
  ID_SETTER_SET,
  ID_SETTER_M1,
  ID_SETTER_M10,
  ID_SETTER_M100,
  ID_SETTER_P1,
  ID_SETTER_P10,
  ID_SETTER_P100,
  ID_SETTER_CURRENT,
  ID_SETTER_HOME,
  ID_SETTER_GC,
  ID_SETTER_L0,
  ID_SETTER_L1,
  ID_SETTER_L2,
  ID_SETTER_L3,
  ID_SETTER_L4,
  ID_SETTER_L5,
  ID_SETTER_L6,
  ID_SETTER_L7,
  ID_SETTER_L8,
  ID_SETTER_L9,
  ID_SETTER_REGULAR,
  ID_SETTER_SCOPE,
  ID_SETTER_SCOPED,
  ID_SETTER_FULL,
  ID_SETTER_DELETE,
  ID_SETTER_INSERT,

  ID_SETTER_GENERAL00,
  ID_SETTER_GENERAL01,
  ID_SETTER_GENERAL02,
  ID_SETTER_GENERAL03,
  ID_SETTER_GENERAL04,
  ID_SETTER_GENERAL05,
  ID_SETTER_GENERAL06,
  ID_SETTER_GENERAL07,
  ID_SETTER_GENERAL08,
  ID_SETTER_GENERAL09,
  ID_SETTER_GENERAL10,
  ID_SETTER_GENERAL11,
  ID_SETTER_GENERAL12,
  ID_SETTER_GENERAL13,
  ID_SETTER_GENERAL14,
  ID_SETTER_GENERAL15,
  ID_SETTER_GENERAL16,
  ID_SETTER_GENERAL17,
  ID_SETTER_GENERAL18,
  ID_SETTER_GENERAL19,
  ID_SETTER_GENERAL20,
  ID_SETTER_GENERAL21,
  ID_SETTER_GENERAL22,
  ID_SETTER_GENERAL23,
  ID_SETTER_GENERAL24,
  ID_SETTER_GENERAL25,
  ID_SETTER_GENERAL26,
  ID_SETTER_GENERAL27,
  ID_SETTER_GENERAL28,
  ID_SETTER_GENERAL29,
  ID_SETTER_GENERAL30,
  ID_SETTER_GENERAL31,
  ID_SETTER_GENERAL32,
  ID_SETTER_GENERAL33,
  ID_SETTER_GENERAL34,
  ID_SETTER_GENERAL35,
  ID_SETTER_GENERAL36,
  ID_SETTER_GENERAL37,
  ID_SETTER_GENERAL38,
  ID_SETTER_GENERAL39,
  ID_SETTER_GENERAL40,
  ID_SETTER_GENERAL41,
  ID_SETTER_GENERAL42,
  ID_SETTER_GENERAL43,
  ID_SETTER_GENERAL44,
  ID_SETTER_GENERAL45,
  ID_SETTER_GENERAL46,
  ID_SETTER_GENERAL47,
  ID_SETTER_GENERAL48,
  ID_SETTER_GENERAL49,
  ID_SETTER_GENERAL_PREV,
  ID_SETTER_GENERAL_NEXT,

  ID_SETTER_CRESCENDO_PREV,
  ID_SETTER_CRESCENDO_CURRENT,
  ID_SETTER_CRESCENDO_NEXT,
  ID_SETTER_CRESCENDO_A,
  ID_SETTER_CRESCENDO_B,
  ID_SETTER_CRESCENDO_C,
  ID_SETTER_CRESCENDO_D,
  ID_SETTER_CRESCENDO_OVERRIDE,

  ID_SETTER_TEMPERAMENT_PREV,
  ID_SETTER_TEMPERAMENT_NEXT,
  ID_SETTER_PITCH_M1,
  ID_SETTER_PITCH_M10,
  ID_SETTER_PITCH_M100,
  ID_SETTER_PITCH_P1,
  ID_SETTER_PITCH_P10,
  ID_SETTER_PITCH_P100,
  ID_SETTER_TRANSPOSE_DOWN,
  ID_SETTER_TRANSPOSE_UP,

  ID_SETTER_SAVE,
  ID_SETTER_ON,
};

const struct GOElementCreator::ButtonDefinitionEntry GOSetter::m_element_types[]
  = {
    {wxT("Prev"), ID_SETTER_PREV, true, true, false},
    {wxT("Next"), ID_SETTER_NEXT, true, true, false},
    {wxT("Set"), ID_SETTER_SET, true, false, false},
    {wxT("M1"), ID_SETTER_M1, true, true, false},
    {wxT("M10"), ID_SETTER_M10, true, true, false},
    {wxT("M100"), ID_SETTER_M100, true, true, false},
    {wxT("P1"), ID_SETTER_P1, true, true, false},
    {wxT("P10"), ID_SETTER_P10, true, true, false},
    {wxT("P100"), ID_SETTER_P100, true, true, false},
    {wxT("Current"), ID_SETTER_CURRENT, true, true, false},
    {wxT("Home"), ID_SETTER_HOME, true, true, false},
    {wxT("GC"), ID_SETTER_GC, true, true, false},
    {wxT("L1"), ID_SETTER_L1, true, true, false},
    {wxT("L2"), ID_SETTER_L2, true, true, false},
    {wxT("L3"), ID_SETTER_L3, true, true, false},
    {wxT("L4"), ID_SETTER_L4, true, true, false},
    {wxT("L5"), ID_SETTER_L5, true, true, false},
    {wxT("L6"), ID_SETTER_L6, true, true, false},
    {wxT("L7"), ID_SETTER_L7, true, true, false},
    {wxT("L8"), ID_SETTER_L8, true, true, false},
    {wxT("L9"), ID_SETTER_L9, true, true, false},
    {wxT("L0"), ID_SETTER_L0, true, true, false},
    {wxT("Regular"), ID_SETTER_REGULAR, true, true, false},
    {wxT("Scope"), ID_SETTER_SCOPE, true, true, false},
    {wxT("Scoped"), ID_SETTER_SCOPED, true, true, false},
    {wxT("Full"), ID_SETTER_FULL, true, false, false},
    {wxT("Insert"), ID_SETTER_INSERT, true, true, false},
    {wxT("Delete"), ID_SETTER_DELETE, true, true, false},
    {wxT("General01"), ID_SETTER_GENERAL00, true, true, true},
    {wxT("General02"), ID_SETTER_GENERAL01, true, true, true},
    {wxT("General03"), ID_SETTER_GENERAL02, true, true, true},
    {wxT("General04"), ID_SETTER_GENERAL03, true, true, true},
    {wxT("General05"), ID_SETTER_GENERAL04, true, true, true},
    {wxT("General06"), ID_SETTER_GENERAL05, true, true, true},
    {wxT("General07"), ID_SETTER_GENERAL06, true, true, true},
    {wxT("General08"), ID_SETTER_GENERAL07, true, true, true},
    {wxT("General09"), ID_SETTER_GENERAL08, true, true, true},
    {wxT("General10"), ID_SETTER_GENERAL09, true, true, true},
    {wxT("General11"), ID_SETTER_GENERAL10, true, true, true},
    {wxT("General12"), ID_SETTER_GENERAL11, true, true, true},
    {wxT("General13"), ID_SETTER_GENERAL12, true, true, true},
    {wxT("General14"), ID_SETTER_GENERAL13, true, true, true},
    {wxT("General15"), ID_SETTER_GENERAL14, true, true, true},
    {wxT("General16"), ID_SETTER_GENERAL15, true, true, true},
    {wxT("General17"), ID_SETTER_GENERAL16, true, true, true},
    {wxT("General18"), ID_SETTER_GENERAL17, true, true, true},
    {wxT("General19"), ID_SETTER_GENERAL18, true, true, true},
    {wxT("General20"), ID_SETTER_GENERAL19, true, true, true},
    {wxT("General21"), ID_SETTER_GENERAL20, true, true, true},
    {wxT("General22"), ID_SETTER_GENERAL21, true, true, true},
    {wxT("General23"), ID_SETTER_GENERAL22, true, true, true},
    {wxT("General24"), ID_SETTER_GENERAL23, true, true, true},
    {wxT("General25"), ID_SETTER_GENERAL24, true, true, true},
    {wxT("General26"), ID_SETTER_GENERAL25, true, true, true},
    {wxT("General27"), ID_SETTER_GENERAL26, true, true, true},
    {wxT("General28"), ID_SETTER_GENERAL27, true, true, true},
    {wxT("General29"), ID_SETTER_GENERAL28, true, true, true},
    {wxT("General30"), ID_SETTER_GENERAL29, true, true, true},
    {wxT("General31"), ID_SETTER_GENERAL30, true, true, true},
    {wxT("General32"), ID_SETTER_GENERAL31, true, true, true},
    {wxT("General33"), ID_SETTER_GENERAL32, true, true, true},
    {wxT("General34"), ID_SETTER_GENERAL33, true, true, true},
    {wxT("General35"), ID_SETTER_GENERAL34, true, true, true},
    {wxT("General36"), ID_SETTER_GENERAL35, true, true, true},
    {wxT("General37"), ID_SETTER_GENERAL36, true, true, true},
    {wxT("General38"), ID_SETTER_GENERAL37, true, true, true},
    {wxT("General39"), ID_SETTER_GENERAL38, true, true, true},
    {wxT("General40"), ID_SETTER_GENERAL39, true, true, true},
    {wxT("General41"), ID_SETTER_GENERAL40, true, true, true},
    {wxT("General42"), ID_SETTER_GENERAL41, true, true, true},
    {wxT("General43"), ID_SETTER_GENERAL42, true, true, true},
    {wxT("General44"), ID_SETTER_GENERAL43, true, true, true},
    {wxT("General45"), ID_SETTER_GENERAL44, true, true, true},
    {wxT("General46"), ID_SETTER_GENERAL45, true, true, true},
    {wxT("General47"), ID_SETTER_GENERAL46, true, true, true},
    {wxT("General48"), ID_SETTER_GENERAL47, true, true, true},
    {wxT("General49"), ID_SETTER_GENERAL48, true, true, true},
    {wxT("General50"), ID_SETTER_GENERAL49, true, true, true},

    {wxT("GeneralPrev"), ID_SETTER_GENERAL_PREV, true, true, true},
    {wxT("GeneralNext"), ID_SETTER_GENERAL_NEXT, true, true, true},

    {wxT("PitchP1"), ID_SETTER_PITCH_P1, true, true, false},
    {wxT("PitchP10"), ID_SETTER_PITCH_P10, true, true, false},
    {wxT("PitchP100"), ID_SETTER_PITCH_P100, true, true, false},
    {wxT("PitchM1"), ID_SETTER_PITCH_M1, true, true, false},
    {wxT("PitchM10"), ID_SETTER_PITCH_M10, true, true, false},
    {wxT("PitchM100"), ID_SETTER_PITCH_M100, true, true, false},
    {wxT("TemperamentPrev"), ID_SETTER_TEMPERAMENT_PREV, true, true, false},
    {wxT("TemperamentNext"), ID_SETTER_TEMPERAMENT_NEXT, true, true, false},
    {wxT("TransposeDown"), ID_SETTER_TRANSPOSE_DOWN, true, true, false},
    {wxT("TransposeUp"), ID_SETTER_TRANSPOSE_UP, true, true, false},

    {wxT("Save"), ID_SETTER_SAVE, true, true, false},
    {wxT("OnState"), ID_SETTER_ON, false, true, false},

    {wxT("CrescendoA"), ID_SETTER_CRESCENDO_A, true, true, false},
    {wxT("CrescendoB"), ID_SETTER_CRESCENDO_B, true, true, false},
    {wxT("CrescendoC"), ID_SETTER_CRESCENDO_C, true, true, false},
    {wxT("CrescendoD"), ID_SETTER_CRESCENDO_D, true, true, false},
    {wxT("CrescendoPrev"), ID_SETTER_CRESCENDO_PREV, true, true, false},
    {wxT("CrescendoCurrent"), ID_SETTER_CRESCENDO_CURRENT, true, true, false},
    {wxT("CrescendoNext"), ID_SETTER_CRESCENDO_NEXT, true, true, false},
    {wxT("CrescendoOverride"), ID_SETTER_CRESCENDO_OVERRIDE, true, true, false},
    {wxT(""), -1, false, false, false},
};

const struct GOElementCreator::ButtonDefinitionEntry *GOSetter::
  GetButtonDefinitionList() {
  return m_element_types;
}

GOSetter::GOSetter(GOOrganController *organController)
  : m_OrganController(organController),
    m_pos(0),
    m_bank(0),
    m_crescendopos(0),
    m_crescendobank(0),
    m_framegeneral(0),
    m_general(0),
    m_crescendo(0),
    m_PosDisplay(organController),
    m_BankDisplay(organController),
    m_CrescendoDisplay(organController),
    m_TransposeDisplay(organController),
    m_NameDisplay(organController),
    m_swell(organController),
    m_SetterType(GOCombination::SETTER_REGULAR) {
  CreateButtons(m_OrganController);

  m_buttons[ID_SETTER_PREV]->SetPreconfigIndex(0);
  m_buttons[ID_SETTER_NEXT]->SetPreconfigIndex(1);
  m_buttons[ID_SETTER_SET]->SetPreconfigIndex(2);
  m_buttons[ID_SETTER_CURRENT]->SetPreconfigIndex(3);
  m_buttons[ID_SETTER_GC]->SetPreconfigIndex(4);
  m_buttons[ID_SETTER_M10]->SetPreconfigIndex(5);
  m_buttons[ID_SETTER_P10]->SetPreconfigIndex(6);
  m_buttons[ID_SETTER_L0]->SetPreconfigIndex(7);
  m_buttons[ID_SETTER_L1]->SetPreconfigIndex(8);
  m_buttons[ID_SETTER_L2]->SetPreconfigIndex(9);
  m_buttons[ID_SETTER_L3]->SetPreconfigIndex(10);
  m_buttons[ID_SETTER_L4]->SetPreconfigIndex(11);
  m_buttons[ID_SETTER_L5]->SetPreconfigIndex(12);
  m_buttons[ID_SETTER_L6]->SetPreconfigIndex(13);
  m_buttons[ID_SETTER_L7]->SetPreconfigIndex(14);
  m_buttons[ID_SETTER_L8]->SetPreconfigIndex(15);
  m_buttons[ID_SETTER_L9]->SetPreconfigIndex(16);
  m_buttons[ID_SETTER_PITCH_M1]->SetPreconfigIndex(17);
  m_buttons[ID_SETTER_PITCH_P1]->SetPreconfigIndex(18);
  m_buttons[ID_SETTER_PITCH_M100]->SetPreconfigIndex(19);
  m_buttons[ID_SETTER_PITCH_P100]->SetPreconfigIndex(20);
  m_buttons[ID_SETTER_TEMPERAMENT_PREV]->SetPreconfigIndex(21);
  m_buttons[ID_SETTER_TEMPERAMENT_NEXT]->SetPreconfigIndex(22);
  m_buttons[ID_SETTER_TRANSPOSE_DOWN]->SetPreconfigIndex(23);
  m_buttons[ID_SETTER_TRANSPOSE_UP]->SetPreconfigIndex(24);

  m_buttons[ID_SETTER_PREV]->SetShortcutKey(37);
  m_buttons[ID_SETTER_NEXT]->SetShortcutKey(39);
  m_buttons[ID_SETTER_CURRENT]->SetShortcutKey(40);

  SetSetterType(m_SetterType);
  SetCrescendoType(m_crescendobank);

  m_OrganController->RegisterSoundStateHandler(this);
  m_OrganController->RegisterControlChangedHandler(this);
}

GOSetter::~GOSetter() {}

static const wxString WX_OVERRIDE_MODE = wxT("OverrideMode");

void GOSetter::Load(GOConfigReader &cfg) {
  m_OrganController->RegisterSaveableObject(this);

  wxString buffer;

  m_framegeneral.resize(0);
  for (unsigned i = 0; i < FRAME_GENERALS; i++) {
    m_framegeneral.push_back(new GOGeneralCombination(
      m_OrganController->GetGeneralTemplate(), m_OrganController, true));
    buffer.Printf(wxT("FrameGeneral%03d"), i + 1);
    m_framegeneral[i]->Load(cfg, buffer);
  }

  m_general.resize(0);
  for (unsigned i = 0; i < GENERALS * GENERAL_BANKS; i++) {
    m_general.push_back(new GOGeneralCombination(
      m_OrganController->GetGeneralTemplate(), m_OrganController, true));
    buffer.Printf(wxT("SetterGeneral%03d"), i + 1);
    m_general[i]->Load(cfg, buffer);
  }

  m_crescendo.resize(0);
  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    buffer.Printf(wxT("SetterCrescendo%d"), i);

    bool defaultAddMode
      = cfg.ReadBoolean(ODFSetting, buffer, WX_OVERRIDE_MODE, false, true);

    m_CrescendoOverrideMode[i] = cfg.ReadBoolean(
      CMBSetting, buffer, WX_OVERRIDE_MODE, false, defaultAddMode);
  }
  for (unsigned i = 0; i < N_CRESCENDOS * CRESCENDO_STEPS; i++) {
    m_crescendo.push_back(new GOGeneralCombination(
      m_OrganController->GetGeneralTemplate(), m_OrganController, true));
    m_CrescendoExtraSets.emplace_back();
    buffer.Printf(
      wxT("SetterCrescendo%d_%03d"),
      (i / CRESCENDO_STEPS) + 1,
      (i % CRESCENDO_STEPS) + 1);
    m_crescendo[i]->Load(cfg, buffer);
  }

  m_buttons[ID_SETTER_PREV]->Init(cfg, wxT("SetterPrev"), _("Previous"));
  m_buttons[ID_SETTER_NEXT]->Init(cfg, wxT("SetterNext"), _("Next"));
  m_buttons[ID_SETTER_SET]->Init(cfg, wxT("SetterSet"), _("Set"));
  m_buttons[ID_SETTER_M1]->Init(cfg, wxT("SetterM1"), _("-1"));
  m_buttons[ID_SETTER_M10]->Init(cfg, wxT("SetterM10"), _("-10"));
  m_buttons[ID_SETTER_M100]->Init(cfg, wxT("SetterM100"), _("-100"));
  m_buttons[ID_SETTER_P1]->Init(cfg, wxT("SetterP1"), _("+1"));
  m_buttons[ID_SETTER_P10]->Init(cfg, wxT("SetterP10"), _("+10"));
  m_buttons[ID_SETTER_P100]->Init(cfg, wxT("SetterP100"), _("+100"));
  m_buttons[ID_SETTER_CURRENT]->Init(cfg, wxT("SetterCurrent"), _("Current"));
  m_buttons[ID_SETTER_HOME]->Init(cfg, wxT("SetterHome"), _("000"));
  m_buttons[ID_SETTER_GC]->Init(cfg, wxT("SetterGC"), _("G.C."));

  m_buttons[ID_SETTER_REGULAR]->Init(cfg, wxT("SetterRegular"), _("Regular"));
  m_buttons[ID_SETTER_SCOPE]->Init(cfg, wxT("SetterScope"), _("Scope"));
  m_buttons[ID_SETTER_SCOPED]->Init(cfg, wxT("SetterScoped"), _("Scoped"));
  m_buttons[ID_SETTER_FULL]->Init(cfg, wxT("SetterFull"), _("Full"));

  m_buttons[ID_SETTER_INSERT]->Init(cfg, wxT("SetterInsert"), _("Insert"));
  m_buttons[ID_SETTER_DELETE]->Init(cfg, wxT("SetterDelete"), _("Delete"));

  m_buttons[ID_SETTER_CRESCENDO_PREV]->Init(
    cfg, wxT("SetterCrescendoPrev"), _("<"));
  m_buttons[ID_SETTER_CRESCENDO_NEXT]->Init(
    cfg, wxT("SetterCrescendoNext"), _(">"));
  m_buttons[ID_SETTER_CRESCENDO_CURRENT]->Init(
    cfg, wxT("SetterCrescendoCurrent"), _("Current"));
  m_buttons[ID_SETTER_CRESCENDO_A]->Init(cfg, wxT("SetterCrescendoA"), _("A"));
  m_buttons[ID_SETTER_CRESCENDO_B]->Init(cfg, wxT("SetterCrescendoB"), _("B"));
  m_buttons[ID_SETTER_CRESCENDO_C]->Init(cfg, wxT("SetterCrescendoC"), _("C"));
  m_buttons[ID_SETTER_CRESCENDO_D]->Init(cfg, wxT("SetterCrescendoD"), _("D"));
  m_buttons[ID_SETTER_CRESCENDO_OVERRIDE]->Init(
    cfg, wxT("SetterCrescendoOverride"), _("Override"));
  m_buttons[ID_SETTER_CRESCENDO_OVERRIDE]->Display(
    m_CrescendoOverrideMode[m_crescendobank]);

  m_buttons[ID_SETTER_PITCH_M1]->Init(cfg, wxT("SetterPitchM1"), _("-1"));
  m_buttons[ID_SETTER_PITCH_M10]->Init(cfg, wxT("SetterPitchM10"), _("-10"));
  m_buttons[ID_SETTER_PITCH_M100]->Init(cfg, wxT("SetterPitchM100"), _("-100"));
  m_buttons[ID_SETTER_PITCH_P1]->Init(cfg, wxT("SetterPitchP1"), _("+1"));
  m_buttons[ID_SETTER_PITCH_P10]->Init(cfg, wxT("SetterPitchP10"), _("+10"));
  m_buttons[ID_SETTER_PITCH_P100]->Init(cfg, wxT("SetterPitchP100"), _("+100"));
  m_buttons[ID_SETTER_TEMPERAMENT_PREV]->Init(
    cfg, wxT("SetterTemperamentPrev"), _("<"));
  m_buttons[ID_SETTER_TEMPERAMENT_NEXT]->Init(
    cfg, wxT("SetterTemperamentNext"), _(">"));
  m_buttons[ID_SETTER_TRANSPOSE_DOWN]->Init(
    cfg, wxT("SetterTransposeDown"), _("-"));
  m_buttons[ID_SETTER_TRANSPOSE_UP]->Init(
    cfg, wxT("SetterTransposeUp"), _("+"));

  m_buttons[ID_SETTER_SAVE]->Init(cfg, wxT("SetterSave"), _("Save"));
  m_buttons[ID_SETTER_ON]->Init(cfg, wxT("SetterOn"), _("ON"));
  m_buttons[ID_SETTER_ON]->Display(true);

  m_swell.Init(cfg, wxT("SetterSwell"), _("Crescendo"), 0);

  m_PosDisplay.Init(cfg, wxT("SetterCurrentPosition"), _("sequencer position"));
  m_BankDisplay.Init(cfg, wxT("SetterGeneralBank"), _("general bank"));
  m_CrescendoDisplay.Init(
    cfg, wxT("SetterCrescendoPosition"), _("crescendo position"));
  m_TransposeDisplay.Init(cfg, wxT("SetterTranspose"), _("transpose"));
  m_NameDisplay.Init(cfg, wxT("SetterName"), _("organ name"));

  for (unsigned i = 0; i < 10; i++) {
    wxString group;
    wxString buffer;
    buffer.Printf(_("__%d"), i);
    group.Printf(wxT("SetterL%d"), i);
    m_buttons[ID_SETTER_L0 + i]->Init(cfg, group, buffer);
  }

  for (unsigned i = 0; i < GENERALS; i++) {
    wxString group;
    wxString buffer;
    buffer.Printf(_("%d"), i + 1);
    group.Printf(wxT("SetterGeneral%d"), i);
    m_buttons[ID_SETTER_GENERAL00 + i]->Init(cfg, group, buffer);
  }
  m_buttons[ID_SETTER_GENERAL_PREV]->Init(
    cfg, wxT("SetterGeneralPrev"), _("Prev"));
  m_buttons[ID_SETTER_GENERAL_NEXT]->Init(
    cfg, wxT("SetterGeneralNext"), _("Next"));
}

void GOSetter::NotifyCmbChanged() {
  // Temporary we mark the organ modified when a combination is changed for
  // the user would save the preset.
  // But we intend to split combinations and organ settings, so in the future
  // we will only mark the combinations as modified, not the organ settings
  m_OrganController->SetOrganModified();
}

void GOSetter::NotifyCmbPushed(bool isChanged) {
  if (isChanged && IsSetterActive())
    NotifyCmbChanged();
}

void GOSetter::Save(GOConfigWriter &cfg) {
  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    cfg.WriteBoolean(
      wxString::Format(wxT("SetterCrescendo%d"), i),
      WX_OVERRIDE_MODE,
      m_CrescendoOverrideMode[i]);
  }
  // another objects are saveble themself so they are saved separatelly
}

const char *const CURRENT = "current";
const char *const SIMPLE_GENERALS = "generals";
const char *const BANKED_GENERALS = "banked-generals";
const char *const CRESCENDOS = "crescendos";
const char *const OVERRIDE_MODE = "override-mode";
const char *const STEPS = "steps";
const char *const SEQUENCER = "sequencer";
const wxString WX_C = wxT("%c");
const wxString WX_C02U = wxT("%c%02u");
const wxString WX_U = wxT("%u");
const wxString WX_03U = wxT("%03u");

wxString general_yaml_key(unsigned i) { return wxString::Format(WX_U, i); }

wxString banked_general_yaml_key(unsigned i) {
  return wxString::Format(WX_C02U, i / GENERALS + 'A', i % GENERALS + 1);
}

wxString crescendo_yaml_key(unsigned i) {
  return wxString::Format(WX_C, i + 'A');
}

wxString crescendo_step_yaml_key(unsigned i) {
  return wxString::Format(WX_U, i + 1);
}

wxString sequencer_cmb_yaml_key(unsigned i) {
  return wxString::Format(WX_03U, i);
}

void GOSetter::ToYaml(YAML::Node &yamlNode) const {
  // save generals
  YAML::Node generalsNode;

  for (unsigned l = m_OrganController->GetGeneralCount(), i = 0; i < l; i++)
    m_OrganController->GetGeneral(i)->GetCombination().PutToYamlMap(
      generalsNode, general_yaml_key(i));
  put_to_map_if_not_null(yamlNode, SIMPLE_GENERALS, generalsNode);

  // save banked generals
  YAML::Node bankedGeneralsNode;

  for (unsigned l = m_general.size(), i = 0; i < l; i++)
    GOCombination::putToYamlMap(
      bankedGeneralsNode, banked_general_yaml_key(i), m_general[i]);
  put_to_map_if_not_null(yamlNode, BANKED_GENERALS, bankedGeneralsNode);

  // save crescendos
  YAML::Node crescendosNode;

  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    YAML::Node crescendoSteps;
    unsigned baseIndex = CRESCENDO_STEPS * i;

    for (unsigned j = 0; j < CRESCENDO_STEPS; j++)
      GOCombination::putToYamlMap(
        crescendoSteps, crescendo_step_yaml_key(j), m_crescendo[baseIndex + j]);
    if (!crescendoSteps.IsNull()) {
      YAML::Node crescendoNode = crescendosNode[crescendo_yaml_key(i)];

      crescendoNode[OVERRIDE_MODE] = m_CrescendoOverrideMode[i];
      crescendoNode[STEPS] = crescendoSteps;
    }
  }
  put_to_map_if_not_null(yamlNode, CRESCENDOS, crescendosNode);

  // save sequencer
  YAML::Node sequencerNode;

  for (unsigned i = 0; i < FRAME_GENERALS; i++)
    GOCombination::putToYamlMap(
      sequencerNode, wxString::Format(WX_03U, i), m_framegeneral[i]);
  put_to_map_if_not_null(yamlNode, SEQUENCER, sequencerNode);
}

void GOSetter::FromYaml(const YAML::Node &yamlNode) {
  // restore generals
  const YAML::Node generalsNode = yamlNode[SIMPLE_GENERALS];

  for (unsigned l = m_OrganController->GetGeneralCount(), i = 0; i < l; i++)
    get_from_map_or_null(generalsNode, general_yaml_key(i))
      >> m_OrganController->GetGeneral(i)->GetCombination();

  // restore banked generals
  const YAML::Node bankedGeneralsNode = yamlNode[BANKED_GENERALS];

  for (unsigned l = m_general.size(), i = 0; i < l; i++)
    get_from_map_or_null(bankedGeneralsNode, banked_general_yaml_key(i))
      >> *m_general[i];

  // restore crescendos
  const YAML::Node crescendosNode = yamlNode[CRESCENDOS];

  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    const YAML::Node crescendoNode
      = get_from_map_or_null(crescendosNode, crescendo_yaml_key(i));
    const YAML::Node crescendoOverridNode
      = get_from_map_or_null(crescendoNode, OVERRIDE_MODE);
    const YAML::Node crescendoSteps
      = get_from_map_or_null(crescendoNode, STEPS);
    unsigned baseIndex = CRESCENDO_STEPS * i;

    m_CrescendoOverrideMode[i] = crescendoOverridNode.as<bool, bool>(true);
    for (unsigned j = 0; j < CRESCENDO_STEPS; j++)
      get_from_map_or_null(crescendoSteps, crescendo_step_yaml_key(j))
        >> *m_crescendo[baseIndex + j];
  }

  // restore sequencer
  const YAML::Node sequencerNode = yamlNode[SEQUENCER];

  for (unsigned i = 0; i < FRAME_GENERALS; i++)
    get_from_map_or_null(sequencerNode, sequencer_cmb_yaml_key(i))
      >> *m_framegeneral[i];
}

void GOSetter::ButtonStateChanged(int id) {
  GOCombination::ExtraElementsSet elementSet;

  switch (id) {
  case ID_SETTER_PREV:
    Prev();
    break;
  case ID_SETTER_NEXT:
    Next();
    break;
  case ID_SETTER_SET:
    wxTheApp->GetTopWindow()->UpdateWindowUI();
    break;
  case ID_SETTER_M1:
    SetPosition(m_pos - 1, false);
    break;
  case ID_SETTER_M10:
    SetPosition(m_pos - 10, false);
    break;
  case ID_SETTER_M100:
    SetPosition(m_pos - 100, false);
    break;
  case ID_SETTER_P1:
    SetPosition(m_pos + 1, false);
    break;
  case ID_SETTER_P10:
    SetPosition(m_pos + 10, false);
    break;
  case ID_SETTER_P100:
    SetPosition(m_pos + 100, false);
    break;
  case ID_SETTER_HOME:
    SetPosition(0, false);
    break;
  case ID_SETTER_GC: {
    GOButtonControl *pGc = m_buttons[ID_SETTER_GC];

    pGc->Display(true);
    m_OrganController->Reset();
    pGc->Display(false);
    break;
  }
  case ID_SETTER_CURRENT:
    SetPosition(m_pos);
    break;
  case ID_SETTER_DELETE:
    for (unsigned j = m_pos; j < m_framegeneral.size() - 1; j++)
      m_framegeneral[j]->Copy(m_framegeneral[j + 1]);
    ResetDisplay();
    NotifyCmbChanged();
    break;
  case ID_SETTER_INSERT:
    for (unsigned j = m_framegeneral.size() - 1; j > m_pos; j--)
      m_framegeneral[j]->Copy(m_framegeneral[j - 1]);
    SetPosition(m_pos);
    NotifyCmbChanged();
    break;
  case ID_SETTER_L0:
  case ID_SETTER_L1:
  case ID_SETTER_L2:
  case ID_SETTER_L3:
  case ID_SETTER_L4:
  case ID_SETTER_L5:
  case ID_SETTER_L6:
  case ID_SETTER_L7:
  case ID_SETTER_L8:
  case ID_SETTER_L9:
    SetPosition(m_pos - (m_pos % 10) + id - ID_SETTER_L0);
    break;
  case ID_SETTER_GENERAL00:
  case ID_SETTER_GENERAL01:
  case ID_SETTER_GENERAL02:
  case ID_SETTER_GENERAL03:
  case ID_SETTER_GENERAL04:
  case ID_SETTER_GENERAL05:
  case ID_SETTER_GENERAL06:
  case ID_SETTER_GENERAL07:
  case ID_SETTER_GENERAL08:
  case ID_SETTER_GENERAL09:
  case ID_SETTER_GENERAL10:
  case ID_SETTER_GENERAL11:
  case ID_SETTER_GENERAL12:
  case ID_SETTER_GENERAL13:
  case ID_SETTER_GENERAL14:
  case ID_SETTER_GENERAL15:
  case ID_SETTER_GENERAL16:
  case ID_SETTER_GENERAL17:
  case ID_SETTER_GENERAL18:
  case ID_SETTER_GENERAL19:
  case ID_SETTER_GENERAL20:
  case ID_SETTER_GENERAL21:
  case ID_SETTER_GENERAL22:
  case ID_SETTER_GENERAL23:
  case ID_SETTER_GENERAL24:
  case ID_SETTER_GENERAL25:
  case ID_SETTER_GENERAL26:
  case ID_SETTER_GENERAL27:
  case ID_SETTER_GENERAL28:
  case ID_SETTER_GENERAL29:
  case ID_SETTER_GENERAL30:
  case ID_SETTER_GENERAL31:
  case ID_SETTER_GENERAL32:
  case ID_SETTER_GENERAL33:
  case ID_SETTER_GENERAL34:
  case ID_SETTER_GENERAL35:
  case ID_SETTER_GENERAL36:
  case ID_SETTER_GENERAL37:
  case ID_SETTER_GENERAL38:
  case ID_SETTER_GENERAL39:
  case ID_SETTER_GENERAL40:
  case ID_SETTER_GENERAL41:
  case ID_SETTER_GENERAL42:
  case ID_SETTER_GENERAL43:
  case ID_SETTER_GENERAL44:
  case ID_SETTER_GENERAL45:
  case ID_SETTER_GENERAL46:
  case ID_SETTER_GENERAL47:
  case ID_SETTER_GENERAL48:
  case ID_SETTER_GENERAL49:
    NotifyCmbPushed(
      m_general[id - ID_SETTER_GENERAL00 + m_bank * GENERALS]->Push(
        GetCrescendoAddSet(elementSet)));
    ResetDisplay();
    m_buttons[id]->Display(true);
    break;
  case ID_SETTER_GENERAL_PREV:
  case ID_SETTER_GENERAL_NEXT:
    if (id == ID_SETTER_GENERAL_PREV && m_bank > 0)
      m_bank--;
    if (id == ID_SETTER_GENERAL_NEXT && m_bank < GENERAL_BANKS - 1)
      m_bank++;

    m_BankDisplay.SetContent(wxString::Format(wxT("%c"), m_bank + wxT('A')));
    break;

  case ID_SETTER_REGULAR:
    SetSetterType(GOCombination::SETTER_REGULAR);
    break;
  case ID_SETTER_SCOPE:
    SetSetterType(GOCombination::SETTER_SCOPE);
    break;
  case ID_SETTER_SCOPED:
    SetSetterType(GOCombination::SETTER_SCOPED);
    break;
  case ID_SETTER_CRESCENDO_A:
  case ID_SETTER_CRESCENDO_B:
  case ID_SETTER_CRESCENDO_C:
  case ID_SETTER_CRESCENDO_D:
    SetCrescendoType(id - ID_SETTER_CRESCENDO_A);
    break;

  case ID_SETTER_CRESCENDO_PREV:
    Crescendo(m_crescendopos - 1, true);
    break;
  case ID_SETTER_CRESCENDO_NEXT:
    Crescendo(m_crescendopos + 1, true);
    break;
  case ID_SETTER_CRESCENDO_CURRENT:
    NotifyCmbPushed(
      m_crescendo[m_crescendopos + m_crescendobank * CRESCENDO_STEPS]->Push());
    break;

  case ID_SETTER_CRESCENDO_OVERRIDE: {
    GOButtonControl *btn = m_buttons[ID_SETTER_CRESCENDO_OVERRIDE];
    bool newIsOverride = !btn->IsEngaged();

    m_CrescendoOverrideMode[m_crescendobank] = newIsOverride;
    btn->Display(newIsOverride);
  } break;

  case ID_SETTER_PITCH_M1:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(-1);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(-1);
    break;
  case ID_SETTER_PITCH_M10:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(-10);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(-10);
    break;
  case ID_SETTER_PITCH_M100:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(-100);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(-100);
    break;
  case ID_SETTER_PITCH_P1:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(1);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(1);
    break;
  case ID_SETTER_PITCH_P10:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(10);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(10);
    break;
  case ID_SETTER_PITCH_P100:
    m_OrganController->GetRootPipeConfigNode().ModifyManualTuning(100);
    m_OrganController->GetRootPipeConfigNode().ModifyAutoTuningCorrection(100);
    break;
  case ID_SETTER_SAVE:
    m_OrganController->Save();
    break;

  case ID_SETTER_TEMPERAMENT_NEXT:
  case ID_SETTER_TEMPERAMENT_PREV: {
    unsigned index
      = m_OrganController->GetSettings().GetTemperaments().GetTemperamentIndex(
        m_OrganController->GetTemperament());
    index += m_OrganController->GetSettings()
               .GetTemperaments()
               .GetTemperamentCount();
    if (id == ID_SETTER_TEMPERAMENT_NEXT)
      index++;
    else
      index--;
    index = index
      % m_OrganController->GetSettings()
          .GetTemperaments()
          .GetTemperamentCount();
    m_OrganController->SetTemperament(
      m_OrganController->GetSettings().GetTemperaments().GetTemperamentName(
        index));
  } break;

  case ID_SETTER_TRANSPOSE_DOWN:
  case ID_SETTER_TRANSPOSE_UP: {
    int value = m_OrganController->GetSettings().Transpose();
    if (id == ID_SETTER_TRANSPOSE_UP)
      value++;
    else
      value--;
    SetTranspose(value);
  } break;
  }
}

void GOSetter::PreparePlayback() {
  wxString buffer;
  buffer.Printf(wxT("%03d"), m_pos);
  m_PosDisplay.SetContent(buffer);

  m_NameDisplay.SetContent(m_OrganController->GetChurchName());

  wxCommandEvent event(wxEVT_SETVALUE, ID_METER_FRAME_SPIN);
  event.SetInt(m_pos);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

  buffer.Printf(wxT("%d"), m_crescendopos + 1);
  m_CrescendoDisplay.SetContent(buffer);

  buffer.Printf(wxT("%c"), m_bank + wxT('A'));
  m_BankDisplay.SetContent(buffer);

  UpdateTranspose();
}

void GOSetter::Update() {}

bool GOSetter::IsSetterActive() {
  return m_buttons[ID_SETTER_SET]->IsEngaged();
}

bool GOSetter::StoreInvisibleObjects() {
  return m_buttons[ID_SETTER_FULL]->IsEngaged();
}

void GOSetter::SetterActive(bool on) { m_buttons[ID_SETTER_SET]->Set(on); }

void GOSetter::ToggleSetter() { m_buttons[ID_SETTER_SET]->Push(); }

void GOSetter::Next() { SetPosition(m_pos + 1); }

void GOSetter::Prev() { SetPosition(m_pos - 1); }

void GOSetter::Push() { SetPosition(m_pos); }

unsigned GOSetter::GetPosition() { return m_pos; }

void GOSetter::SetSetterType(GOCombination::SetterType type) {
  m_SetterType = type;
  m_buttons[ID_SETTER_REGULAR]->Display(type == GOCombination::SETTER_REGULAR);
  m_buttons[ID_SETTER_SCOPE]->Display(type == GOCombination::SETTER_SCOPE);
  m_buttons[ID_SETTER_SCOPED]->Display(type == GOCombination::SETTER_SCOPED);
}

void GOSetter::SetCrescendoType(unsigned no) {
  m_crescendobank = no;
  m_buttons[ID_SETTER_CRESCENDO_A]->Display(no == 0);
  m_buttons[ID_SETTER_CRESCENDO_B]->Display(no == 1);
  m_buttons[ID_SETTER_CRESCENDO_C]->Display(no == 2);
  m_buttons[ID_SETTER_CRESCENDO_D]->Display(no == 3);
  m_buttons[ID_SETTER_CRESCENDO_OVERRIDE]->Display(
    m_CrescendoOverrideMode[m_crescendobank]);
}

const GOCombination::ExtraElementsSet *GOSetter::GetCrescendoAddSet(
  GOCombination::ExtraElementsSet &elementSet) {
  const GOCombination::ExtraElementsSet *pResElementSet = nullptr;

  if (!m_CrescendoOverrideMode[m_crescendobank]) {
    m_crescendo[m_crescendopos + m_crescendobank * CRESCENDO_STEPS]
      ->GetEnabledElements(elementSet);
    pResElementSet = &elementSet;
  }
  return pResElementSet;
}

void GOSetter::ResetDisplay() {
  m_buttons[ID_SETTER_HOME]->Display(false);
  for (unsigned i = 0; i < 10; i++)
    m_buttons[ID_SETTER_L0 + i]->Display(false);
  for (unsigned i = 0; i < GENERALS; i++)
    m_buttons[ID_SETTER_GENERAL00 + i]->Display(false);
}

void GOSetter::UpdatePosition(int pos) {
  if (pos != (int)m_pos)
    SetPosition(pos);
}

void GOSetter::SetPosition(int pos, bool push) {
  wxString buffer;
  int old_pos = m_pos;
  while (pos < 0)
    pos += m_framegeneral.size();
  while (pos >= (int)m_framegeneral.size())
    pos -= m_framegeneral.size();
  m_pos = pos;
  if (push) {
    GOCombination::ExtraElementsSet elementSet;

    NotifyCmbPushed(
      m_framegeneral[m_pos]->Push(GetCrescendoAddSet(elementSet)));

    m_buttons[ID_SETTER_HOME]->Display(m_pos == 0);
    for (unsigned i = 0; i < 10; i++)
      m_buttons[ID_SETTER_L0 + i]->Display((m_pos % 10) == i);
    for (unsigned i = 0; i < GENERALS; i++)
      m_buttons[ID_SETTER_GENERAL00 + i]->Display(false);
  }

  buffer.Printf(wxT("%03d"), m_pos);
  m_PosDisplay.SetContent(buffer);
  if (pos != old_pos) {
    wxCommandEvent event(wxEVT_SETVALUE, ID_METER_FRAME_SPIN);
    event.SetInt(m_pos);
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
  }
}

void GOSetter::Crescendo(int newpos, bool force) {
  if (newpos < 0)
    newpos = 0;
  if (newpos > CRESCENDO_STEPS - 1)
    newpos = CRESCENDO_STEPS - 1;
  if (IsSetterActive() && !force)
    return;
  unsigned pos = newpos;
  if (pos == m_crescendopos)
    return;

  bool crescendoAddMode = !m_CrescendoOverrideMode[m_crescendobank];
  bool changed = false;

  while (pos > m_crescendopos) {
    const unsigned oldIdx = m_crescendopos + m_crescendobank * CRESCENDO_STEPS;
    const unsigned newIdx = oldIdx + 1;

    if (crescendoAddMode)
      m_crescendo[oldIdx]->GetExtraSetState(m_CrescendoExtraSets[oldIdx]);
    else
      m_CrescendoExtraSets[oldIdx].clear();
    ++m_crescendopos;
    changed = changed
      || m_crescendo[newIdx]->Push(
        crescendoAddMode ? &m_CrescendoExtraSets[oldIdx] : nullptr, true);
  }

  while (pos < m_crescendopos) {
    --m_crescendopos;

    const unsigned newIdx = m_crescendopos + m_crescendobank * CRESCENDO_STEPS;

    changed = changed
      || m_crescendo[newIdx]->Push(
        crescendoAddMode ? &m_CrescendoExtraSets[newIdx] : nullptr, true);
  }
  NotifyCmbPushed(changed);

  wxString buffer;
  buffer.Printf(wxT("%d"), m_crescendopos + 1);
  m_CrescendoDisplay.SetContent(buffer);
}

void GOSetter::ControlChanged(void *control) {
  if (control == &m_swell)
    Crescendo(m_swell.GetValue() * CRESCENDO_STEPS / 128);
}

void GOSetter::UpdateTranspose() {
  m_TransposeDisplay.SetContent(
    wxString::Format(wxT("%d"), m_OrganController->GetSettings().Transpose()));
}

void GOSetter::SetTranspose(int value) {
  if (m_OrganController->GetSettings().Transpose() != value) {
    wxCommandEvent event(wxEVT_SETVALUE, ID_METER_TRANSPOSE_SPIN);
    event.SetInt(value);
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
  }
  m_OrganController->GetSettings().Transpose(value);
  m_OrganController->AllNotesOff();
  UpdateTranspose();
}

void GOSetter::UpdateModified(bool modified) {
  m_buttons[ID_SETTER_SAVE]->Display(modified);
}

GOEnclosure *GOSetter::GetEnclosure(const wxString &name, bool is_panel) {
  if (name == wxT("Swell"))
    return &m_swell;

  return NULL;
}

GOLabelControl *GOSetter::GetLabelControl(const wxString &name, bool is_panel) {
  if (name == wxT("Label") || name == wxT("SequencerLabel"))
    return &m_PosDisplay;

  if (name == wxT("CrescendoLabel"))
    return &m_CrescendoDisplay;

  if (name == wxT("GeneralLabel"))
    return &m_BankDisplay;

  if (name == wxT("PitchLabel"))
    return m_OrganController->GetPitchLabel();

  if (name == wxT("TemperamentLabel"))
    return m_OrganController->GetTemperamentLabel();

  if (name == wxT("TransposeLabel"))
    return &m_TransposeDisplay;

  if (is_panel)
    return NULL;

  if (name == wxT("OrganNameLabel"))
    return &m_NameDisplay;

  return NULL;
}
