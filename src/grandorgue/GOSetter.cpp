/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSetter.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>

#include "GODefinitionFile.h"
#include "GOEvent.h"
#include "GOFrameGeneral.h"
#include "GOSetterButton.h"
#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
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

const struct ElementListEntry GOSetter::m_element_types[] = {
  {wxT("Prev"), ID_SETTER_PREV, true, true},
  {wxT("Next"), ID_SETTER_NEXT, true, true},
  {wxT("Set"), ID_SETTER_SET, true, false},
  {wxT("M1"), ID_SETTER_M1, true, true},
  {wxT("M10"), ID_SETTER_M10, true, true},
  {wxT("M100"), ID_SETTER_M100, true, true},
  {wxT("P1"), ID_SETTER_P1, true, true},
  {wxT("P10"), ID_SETTER_P10, true, true},
  {wxT("P100"), ID_SETTER_P100, true, true},
  {wxT("Current"), ID_SETTER_CURRENT, true, true},
  {wxT("Home"), ID_SETTER_HOME, true, true},
  {wxT("GC"), ID_SETTER_GC, true, true},
  {wxT("L1"), ID_SETTER_L1, true, true},
  {wxT("L2"), ID_SETTER_L2, true, true},
  {wxT("L3"), ID_SETTER_L3, true, true},
  {wxT("L4"), ID_SETTER_L4, true, true},
  {wxT("L5"), ID_SETTER_L5, true, true},
  {wxT("L6"), ID_SETTER_L6, true, true},
  {wxT("L7"), ID_SETTER_L7, true, true},
  {wxT("L8"), ID_SETTER_L8, true, true},
  {wxT("L9"), ID_SETTER_L9, true, true},
  {wxT("L0"), ID_SETTER_L0, true, true},
  {wxT("Regular"), ID_SETTER_REGULAR, true, true},
  {wxT("Scope"), ID_SETTER_SCOPE, true, true},
  {wxT("Scoped"), ID_SETTER_SCOPED, true, true},
  {wxT("Full"), ID_SETTER_FULL, true, false},
  {wxT("Insert"), ID_SETTER_INSERT, true, true},
  {wxT("Delete"), ID_SETTER_DELETE, true, true},
  {wxT("General01"), ID_SETTER_GENERAL00, true, true},
  {wxT("General02"), ID_SETTER_GENERAL01, true, true},
  {wxT("General03"), ID_SETTER_GENERAL02, true, true},
  {wxT("General04"), ID_SETTER_GENERAL03, true, true},
  {wxT("General05"), ID_SETTER_GENERAL04, true, true},
  {wxT("General06"), ID_SETTER_GENERAL05, true, true},
  {wxT("General07"), ID_SETTER_GENERAL06, true, true},
  {wxT("General08"), ID_SETTER_GENERAL07, true, true},
  {wxT("General09"), ID_SETTER_GENERAL08, true, true},
  {wxT("General10"), ID_SETTER_GENERAL09, true, true},
  {wxT("General11"), ID_SETTER_GENERAL10, true, true},
  {wxT("General12"), ID_SETTER_GENERAL11, true, true},
  {wxT("General13"), ID_SETTER_GENERAL12, true, true},
  {wxT("General14"), ID_SETTER_GENERAL13, true, true},
  {wxT("General15"), ID_SETTER_GENERAL14, true, true},
  {wxT("General16"), ID_SETTER_GENERAL15, true, true},
  {wxT("General17"), ID_SETTER_GENERAL16, true, true},
  {wxT("General18"), ID_SETTER_GENERAL17, true, true},
  {wxT("General19"), ID_SETTER_GENERAL18, true, true},
  {wxT("General20"), ID_SETTER_GENERAL19, true, true},
  {wxT("General21"), ID_SETTER_GENERAL20, true, true},
  {wxT("General22"), ID_SETTER_GENERAL21, true, true},
  {wxT("General23"), ID_SETTER_GENERAL22, true, true},
  {wxT("General24"), ID_SETTER_GENERAL23, true, true},
  {wxT("General25"), ID_SETTER_GENERAL24, true, true},
  {wxT("General26"), ID_SETTER_GENERAL25, true, true},
  {wxT("General27"), ID_SETTER_GENERAL26, true, true},
  {wxT("General28"), ID_SETTER_GENERAL27, true, true},
  {wxT("General29"), ID_SETTER_GENERAL28, true, true},
  {wxT("General30"), ID_SETTER_GENERAL29, true, true},
  {wxT("General31"), ID_SETTER_GENERAL30, true, true},
  {wxT("General32"), ID_SETTER_GENERAL31, true, true},
  {wxT("General33"), ID_SETTER_GENERAL32, true, true},
  {wxT("General34"), ID_SETTER_GENERAL33, true, true},
  {wxT("General35"), ID_SETTER_GENERAL34, true, true},
  {wxT("General36"), ID_SETTER_GENERAL35, true, true},
  {wxT("General37"), ID_SETTER_GENERAL36, true, true},
  {wxT("General38"), ID_SETTER_GENERAL37, true, true},
  {wxT("General39"), ID_SETTER_GENERAL38, true, true},
  {wxT("General40"), ID_SETTER_GENERAL39, true, true},
  {wxT("General41"), ID_SETTER_GENERAL40, true, true},
  {wxT("General42"), ID_SETTER_GENERAL41, true, true},
  {wxT("General43"), ID_SETTER_GENERAL42, true, true},
  {wxT("General44"), ID_SETTER_GENERAL43, true, true},
  {wxT("General45"), ID_SETTER_GENERAL44, true, true},
  {wxT("General46"), ID_SETTER_GENERAL45, true, true},
  {wxT("General47"), ID_SETTER_GENERAL46, true, true},
  {wxT("General48"), ID_SETTER_GENERAL47, true, true},
  {wxT("General49"), ID_SETTER_GENERAL48, true, true},
  {wxT("General50"), ID_SETTER_GENERAL49, true, true},

  {wxT("GeneralPrev"), ID_SETTER_GENERAL_PREV, true, true},
  {wxT("GeneralNext"), ID_SETTER_GENERAL_NEXT, true, true},

  {wxT("PitchP1"), ID_SETTER_PITCH_P1, true, true},
  {wxT("PitchP10"), ID_SETTER_PITCH_P10, true, true},
  {wxT("PitchP100"), ID_SETTER_PITCH_P100, true, true},
  {wxT("PitchM1"), ID_SETTER_PITCH_M1, true, true},
  {wxT("PitchM10"), ID_SETTER_PITCH_M10, true, true},
  {wxT("PitchM100"), ID_SETTER_PITCH_M100, true, true},
  {wxT("TemperamentPrev"), ID_SETTER_TEMPERAMENT_PREV, true, true},
  {wxT("TemperamentNext"), ID_SETTER_TEMPERAMENT_NEXT, true, true},
  {wxT("TransposeDown"), ID_SETTER_TRANSPOSE_DOWN, true, true},
  {wxT("TransposeUp"), ID_SETTER_TRANSPOSE_UP, true, true},

  {wxT("Save"), ID_SETTER_SAVE, true, true},
  {wxT("OnState"), ID_SETTER_ON, false, true},

  {wxT("CrescendoA"), ID_SETTER_CRESCENDO_A, true, true},
  {wxT("CrescendoB"), ID_SETTER_CRESCENDO_B, true, true},
  {wxT("CrescendoC"), ID_SETTER_CRESCENDO_C, true, true},
  {wxT("CrescendoD"), ID_SETTER_CRESCENDO_D, true, true},
  {wxT("CrescendoPrev"), ID_SETTER_CRESCENDO_PREV, true, true},
  {wxT("CrescendoCurrent"), ID_SETTER_CRESCENDO_CURRENT, true, true},
  {wxT("CrescendoNext"), ID_SETTER_CRESCENDO_NEXT, true, true},
  {wxT("CrescendoOverride"), ID_SETTER_CRESCENDO_OVERRIDE, true, true},
  {wxT(""), -1, false, false},
};

const struct ElementListEntry *GOSetter::GetButtonList() {
  return m_element_types;
}

GOSetter::GOSetter(GODefinitionFile *organfile)
  : m_organfile(organfile),
    m_pos(0),
    m_bank(0),
    m_crescendopos(0),
    m_crescendobank(0),
    m_framegeneral(0),
    m_general(0),
    m_crescendo(0),
    m_PosDisplay(organfile),
    m_BankDisplay(organfile),
    m_CrescendoDisplay(organfile),
    m_TransposeDisplay(organfile),
    m_NameDisplay(organfile),
    m_swell(organfile),
    m_SetterType(SETTER_REGULAR) {
  CreateButtons(m_organfile);

  m_button[ID_SETTER_PREV]->SetPreconfigIndex(0);
  m_button[ID_SETTER_NEXT]->SetPreconfigIndex(1);
  m_button[ID_SETTER_SET]->SetPreconfigIndex(2);
  m_button[ID_SETTER_CURRENT]->SetPreconfigIndex(3);
  m_button[ID_SETTER_GC]->SetPreconfigIndex(4);
  m_button[ID_SETTER_M10]->SetPreconfigIndex(5);
  m_button[ID_SETTER_P10]->SetPreconfigIndex(6);
  m_button[ID_SETTER_L0]->SetPreconfigIndex(7);
  m_button[ID_SETTER_L1]->SetPreconfigIndex(8);
  m_button[ID_SETTER_L2]->SetPreconfigIndex(9);
  m_button[ID_SETTER_L3]->SetPreconfigIndex(10);
  m_button[ID_SETTER_L4]->SetPreconfigIndex(11);
  m_button[ID_SETTER_L5]->SetPreconfigIndex(12);
  m_button[ID_SETTER_L6]->SetPreconfigIndex(13);
  m_button[ID_SETTER_L7]->SetPreconfigIndex(14);
  m_button[ID_SETTER_L8]->SetPreconfigIndex(15);
  m_button[ID_SETTER_L9]->SetPreconfigIndex(16);
  m_button[ID_SETTER_PITCH_M1]->SetPreconfigIndex(17);
  m_button[ID_SETTER_PITCH_P1]->SetPreconfigIndex(18);
  m_button[ID_SETTER_PITCH_M100]->SetPreconfigIndex(19);
  m_button[ID_SETTER_PITCH_P100]->SetPreconfigIndex(20);
  m_button[ID_SETTER_TEMPERAMENT_PREV]->SetPreconfigIndex(21);
  m_button[ID_SETTER_TEMPERAMENT_NEXT]->SetPreconfigIndex(22);
  m_button[ID_SETTER_TRANSPOSE_DOWN]->SetPreconfigIndex(23);
  m_button[ID_SETTER_TRANSPOSE_UP]->SetPreconfigIndex(24);

  m_button[ID_SETTER_PREV]->SetShortcutKey(37);
  m_button[ID_SETTER_NEXT]->SetShortcutKey(39);
  m_button[ID_SETTER_CURRENT]->SetShortcutKey(40);

  SetSetterType(m_SetterType);
  SetCrescendoType(m_crescendobank);

  m_organfile->RegisterPlaybackStateHandler(this);
  m_organfile->RegisterControlChangedHandler(this);
}

GOSetter::~GOSetter() {}

static const wxString OVERRIDE_MODE = wxT("OverrideMode");

void GOSetter::Load(GOConfigReader &cfg) {
  m_organfile->RegisterSaveableObject(this);

  wxString buffer;

  m_framegeneral.resize(0);
  for (unsigned i = 0; i < FRAME_GENERALS; i++) {
    m_framegeneral.push_back(
      new GOFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
    buffer.Printf(wxT("FrameGeneral%03d"), i + 1);
    m_framegeneral[i]->Load(cfg, buffer);
  }

  m_general.resize(0);
  for (unsigned i = 0; i < GENERALS * GENERAL_BANKS; i++) {
    m_general.push_back(
      new GOFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
    buffer.Printf(wxT("SetterGeneral%03d"), i + 1);
    m_general[i]->Load(cfg, buffer);
  }

  m_crescendo.resize(0);
  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    buffer.Printf(wxT("SetterCrescendo%d"), i);

    bool defaultAddMode
      = cfg.ReadBoolean(ODFSetting, buffer, OVERRIDE_MODE, false, true);

    m_CrescendoOverrideMode[i] = cfg.ReadBoolean(
      CMBSetting, buffer, OVERRIDE_MODE, false, defaultAddMode);
  }
  for (unsigned i = 0; i < N_CRESCENDOS * CRESCENDO_STEPS; i++) {
    m_crescendo.push_back(
      new GOFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
    m_CrescendoExtraSets.emplace_back();
    buffer.Printf(
      wxT("SetterCrescendo%d_%03d"),
      (i / CRESCENDO_STEPS) + 1,
      (i % CRESCENDO_STEPS) + 1);
    m_crescendo[i]->Load(cfg, buffer);
  }

  m_button[ID_SETTER_PREV]->Init(cfg, wxT("SetterPrev"), _("Previous"));
  m_button[ID_SETTER_NEXT]->Init(cfg, wxT("SetterNext"), _("Next"));
  m_button[ID_SETTER_SET]->Init(cfg, wxT("SetterSet"), _("Set"));
  m_button[ID_SETTER_M1]->Init(cfg, wxT("SetterM1"), _("-1"));
  m_button[ID_SETTER_M10]->Init(cfg, wxT("SetterM10"), _("-10"));
  m_button[ID_SETTER_M100]->Init(cfg, wxT("SetterM100"), _("-100"));
  m_button[ID_SETTER_P1]->Init(cfg, wxT("SetterP1"), _("+1"));
  m_button[ID_SETTER_P10]->Init(cfg, wxT("SetterP10"), _("+10"));
  m_button[ID_SETTER_P100]->Init(cfg, wxT("SetterP100"), _("+100"));
  m_button[ID_SETTER_CURRENT]->Init(cfg, wxT("SetterCurrent"), _("Current"));
  m_button[ID_SETTER_HOME]->Init(cfg, wxT("SetterHome"), _("000"));
  m_button[ID_SETTER_GC]->Init(cfg, wxT("SetterGC"), _("G.C."));

  m_button[ID_SETTER_REGULAR]->Init(cfg, wxT("SetterRegular"), _("Regular"));
  m_button[ID_SETTER_SCOPE]->Init(cfg, wxT("SetterScope"), _("Scope"));
  m_button[ID_SETTER_SCOPED]->Init(cfg, wxT("SetterScoped"), _("Scoped"));
  m_button[ID_SETTER_FULL]->Init(cfg, wxT("SetterFull"), _("Full"));

  m_button[ID_SETTER_INSERT]->Init(cfg, wxT("SetterInsert"), _("Insert"));
  m_button[ID_SETTER_DELETE]->Init(cfg, wxT("SetterDelete"), _("Delete"));

  m_button[ID_SETTER_CRESCENDO_PREV]->Init(
    cfg, wxT("SetterCrescendoPrev"), _("<"));
  m_button[ID_SETTER_CRESCENDO_NEXT]->Init(
    cfg, wxT("SetterCrescendoNext"), _(">"));
  m_button[ID_SETTER_CRESCENDO_CURRENT]->Init(
    cfg, wxT("SetterCrescendoCurrent"), _("Current"));
  m_button[ID_SETTER_CRESCENDO_A]->Init(cfg, wxT("SetterCrescendoA"), _("A"));
  m_button[ID_SETTER_CRESCENDO_B]->Init(cfg, wxT("SetterCrescendoB"), _("B"));
  m_button[ID_SETTER_CRESCENDO_C]->Init(cfg, wxT("SetterCrescendoC"), _("C"));
  m_button[ID_SETTER_CRESCENDO_D]->Init(cfg, wxT("SetterCrescendoD"), _("D"));
  m_button[ID_SETTER_CRESCENDO_OVERRIDE]->Init(
    cfg, wxT("SetterCrescendoOverride"), _("Override"));
  m_button[ID_SETTER_CRESCENDO_OVERRIDE]->Display(
    m_CrescendoOverrideMode[m_crescendobank]);

  m_button[ID_SETTER_PITCH_M1]->Init(cfg, wxT("SetterPitchM1"), _("-1"));
  m_button[ID_SETTER_PITCH_M10]->Init(cfg, wxT("SetterPitchM10"), _("-10"));
  m_button[ID_SETTER_PITCH_M100]->Init(cfg, wxT("SetterPitchM100"), _("-100"));
  m_button[ID_SETTER_PITCH_P1]->Init(cfg, wxT("SetterPitchP1"), _("+1"));
  m_button[ID_SETTER_PITCH_P10]->Init(cfg, wxT("SetterPitchP10"), _("+10"));
  m_button[ID_SETTER_PITCH_P100]->Init(cfg, wxT("SetterPitchP100"), _("+100"));
  m_button[ID_SETTER_TEMPERAMENT_PREV]->Init(
    cfg, wxT("SetterTemperamentPrev"), _("<"));
  m_button[ID_SETTER_TEMPERAMENT_NEXT]->Init(
    cfg, wxT("SetterTemperamentNext"), _(">"));
  m_button[ID_SETTER_TRANSPOSE_DOWN]->Init(
    cfg, wxT("SetterTransposeDown"), _("-"));
  m_button[ID_SETTER_TRANSPOSE_UP]->Init(cfg, wxT("SetterTransposeUp"), _("+"));

  m_button[ID_SETTER_SAVE]->Init(cfg, wxT("SetterSave"), _("Save"));
  m_button[ID_SETTER_ON]->Init(cfg, wxT("SetterOn"), _("ON"));
  m_button[ID_SETTER_ON]->Display(true);

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
    m_button[ID_SETTER_L0 + i]->Init(cfg, group, buffer);
  }

  for (unsigned i = 0; i < GENERALS; i++) {
    wxString group;
    wxString buffer;
    buffer.Printf(_("%d"), i + 1);
    group.Printf(wxT("SetterGeneral%d"), i);
    m_button[ID_SETTER_GENERAL00 + i]->Init(cfg, group, buffer);
  }
  m_button[ID_SETTER_GENERAL_PREV]->Init(
    cfg, wxT("SetterGeneralPrev"), _("Prev"));
  m_button[ID_SETTER_GENERAL_NEXT]->Init(
    cfg, wxT("SetterGeneralNext"), _("Next"));
}

void GOSetter::Save(GOConfigWriter &cfg) {
  for (unsigned i = 0; i < N_CRESCENDOS; i++) {
    cfg.WriteBoolean(
      wxString::Format(wxT("SetterCrescendo%d"), i),
      OVERRIDE_MODE,
      m_CrescendoOverrideMode[i]);
  }
  // another objects are saveble themself so they are saved separatelly
}

void GOSetter::ButtonChanged(int id) {
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
  case ID_SETTER_GC:
    m_organfile->Reset();
    break;
  case ID_SETTER_CURRENT:
    SetPosition(m_pos);
    break;
  case ID_SETTER_DELETE:
    for (unsigned j = m_pos; j < m_framegeneral.size() - 1; j++)
      m_framegeneral[j]->Copy(m_framegeneral[j + 1]);
    ResetDisplay();
    break;
  case ID_SETTER_INSERT:
    for (unsigned j = m_framegeneral.size() - 1; j > m_pos; j--)
      m_framegeneral[j]->Copy(m_framegeneral[j - 1]);
    SetPosition(m_pos);
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
    m_general[id - ID_SETTER_GENERAL00 + m_bank * GENERALS]->Push();
    ResetDisplay();
    m_button[id]->Display(true);
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
    SetSetterType(SETTER_REGULAR);
    break;
  case ID_SETTER_SCOPE:
    SetSetterType(SETTER_SCOPE);
    break;
  case ID_SETTER_SCOPED:
    SetSetterType(SETTER_SCOPED);
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
    m_crescendo[m_crescendopos + m_crescendobank * CRESCENDO_STEPS]->Push();
    break;

  case ID_SETTER_CRESCENDO_OVERRIDE: {
    GOButton *btn = m_button[ID_SETTER_CRESCENDO_OVERRIDE];
    bool newIsOverride = !btn->IsEngaged();

    m_CrescendoOverrideMode[m_crescendobank] = newIsOverride;
    btn->Display(newIsOverride);
  } break;

  case ID_SETTER_PITCH_M1:
    m_organfile->GetPipeConfig().ModifyTuning(-1);
    break;
  case ID_SETTER_PITCH_M10:
    m_organfile->GetPipeConfig().ModifyTuning(-10);
    break;
  case ID_SETTER_PITCH_M100:
    m_organfile->GetPipeConfig().ModifyTuning(-100);
    break;
  case ID_SETTER_PITCH_P1:
    m_organfile->GetPipeConfig().ModifyTuning(1);
    break;
  case ID_SETTER_PITCH_P10:
    m_organfile->GetPipeConfig().ModifyTuning(10);
    break;
  case ID_SETTER_PITCH_P100:
    m_organfile->GetPipeConfig().ModifyTuning(100);
    break;
  case ID_SETTER_SAVE:
    m_organfile->Save();
    break;

  case ID_SETTER_TEMPERAMENT_NEXT:
  case ID_SETTER_TEMPERAMENT_PREV: {
    unsigned index
      = m_organfile->GetSettings().GetTemperaments().GetTemperamentIndex(
        m_organfile->GetTemperament());
    index += m_organfile->GetSettings().GetTemperaments().GetTemperamentCount();
    if (id == ID_SETTER_TEMPERAMENT_NEXT)
      index++;
    else
      index--;
    index = index
      % m_organfile->GetSettings().GetTemperaments().GetTemperamentCount();
    m_organfile->SetTemperament(
      m_organfile->GetSettings().GetTemperaments().GetTemperamentName(index));
  } break;

  case ID_SETTER_TRANSPOSE_DOWN:
  case ID_SETTER_TRANSPOSE_UP: {
    int value = m_organfile->GetSettings().Transpose();
    if (id == ID_SETTER_TRANSPOSE_UP)
      value++;
    else
      value--;
    SetTranspose(value);
  } break;
  }
}

void GOSetter::AbortPlayback() {}

void GOSetter::PreparePlayback() {
  wxString buffer;
  buffer.Printf(wxT("%03d"), m_pos);
  m_PosDisplay.SetContent(buffer);

  m_NameDisplay.SetContent(m_organfile->GetChurchName());

  wxCommandEvent event(wxEVT_SETVALUE, ID_METER_FRAME_SPIN);
  event.SetInt(m_pos);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

  buffer.Printf(wxT("%d"), m_crescendopos + 1);
  m_CrescendoDisplay.SetContent(buffer);

  buffer.Printf(wxT("%c"), m_bank + wxT('A'));
  m_BankDisplay.SetContent(buffer);

  UpdateTranspose();
}

void GOSetter::StartPlayback() {}

void GOSetter::PrepareRecording() {}

void GOSetter::Update() {}

bool GOSetter::IsSetterActive() { return m_button[ID_SETTER_SET]->IsEngaged(); }

bool GOSetter::StoreInvisibleObjects() {
  return m_button[ID_SETTER_FULL]->IsEngaged();
}

void GOSetter::SetterActive(bool on) { m_button[ID_SETTER_SET]->Set(on); }

void GOSetter::ToggleSetter() { m_button[ID_SETTER_SET]->Push(); }

void GOSetter::Next() { SetPosition(m_pos + 1); }

void GOSetter::Prev() { SetPosition(m_pos - 1); }

void GOSetter::Push() { SetPosition(m_pos); }

unsigned GOSetter::GetPosition() { return m_pos; }

SetterType GOSetter::GetSetterType() { return m_SetterType; }

void GOSetter::SetSetterType(SetterType type) {
  m_SetterType = type;
  m_button[ID_SETTER_REGULAR]->Display(type == SETTER_REGULAR);
  m_button[ID_SETTER_SCOPE]->Display(type == SETTER_SCOPE);
  m_button[ID_SETTER_SCOPED]->Display(type == SETTER_SCOPED);
}

void GOSetter::SetCrescendoType(unsigned no) {
  m_crescendobank = no;
  m_button[ID_SETTER_CRESCENDO_A]->Display(no == 0);
  m_button[ID_SETTER_CRESCENDO_B]->Display(no == 1);
  m_button[ID_SETTER_CRESCENDO_C]->Display(no == 2);
  m_button[ID_SETTER_CRESCENDO_D]->Display(no == 3);
  m_button[ID_SETTER_CRESCENDO_OVERRIDE]->Display(
    m_CrescendoOverrideMode[m_crescendobank]);
}

void GOSetter::ResetDisplay() {
  m_button[ID_SETTER_HOME]->Display(false);
  for (unsigned i = 0; i < 10; i++)
    m_button[ID_SETTER_L0 + i]->Display(false);
  for (unsigned i = 0; i < GENERALS; i++)
    m_button[ID_SETTER_GENERAL00 + i]->Display(false);
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
    m_framegeneral[m_pos]->Push();

    m_button[ID_SETTER_HOME]->Display(m_pos == 0);
    for (unsigned i = 0; i < 10; i++)
      m_button[ID_SETTER_L0 + i]->Display((m_pos % 10) == i);
    for (unsigned i = 0; i < GENERALS; i++)
      m_button[ID_SETTER_GENERAL00 + i]->Display(false);
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

  while (pos > m_crescendopos) {
    const unsigned oldIdx = m_crescendopos + m_crescendobank * CRESCENDO_STEPS;
    const unsigned newIdx = oldIdx + 1;

    if (crescendoAddMode)
      m_crescendo[oldIdx]->GetExtraSetState(m_CrescendoExtraSets[oldIdx]);
    else
      m_CrescendoExtraSets[oldIdx].clear();
    ++m_crescendopos;
    m_crescendo[newIdx]->Push(
      crescendoAddMode ? &m_CrescendoExtraSets[oldIdx] : nullptr);
  }

  while (pos < m_crescendopos) {
    --m_crescendopos;

    const unsigned newIdx = m_crescendopos + m_crescendobank * CRESCENDO_STEPS;

    m_crescendo[newIdx]->Push(
      crescendoAddMode ? &m_CrescendoExtraSets[newIdx] : nullptr);
  }

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
    wxString::Format(wxT("%d"), m_organfile->GetSettings().Transpose()));
}

void GOSetter::SetTranspose(int value) {
  if (m_organfile->GetSettings().Transpose() != value) {
    wxCommandEvent event(wxEVT_SETVALUE, ID_METER_TRANSPOSE_SPIN);
    event.SetInt(value);
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
  }
  m_organfile->GetSettings().Transpose(value);
  m_organfile->AllNotesOff();
  UpdateTranspose();
}

void GOSetter::UpdateModified(bool modified) {
  m_button[ID_SETTER_SAVE]->Display(modified);
}

GOEnclosure *GOSetter::GetEnclosure(const wxString &name, bool is_panel) {
  if (name == wxT("Swell"))
    return &m_swell;

  return NULL;
}

GOLabel *GOSetter::GetLabel(const wxString &name, bool is_panel) {
  if (name == wxT("Label") || name == wxT("SequencerLabel"))
    return &m_PosDisplay;

  if (name == wxT("CrescendoLabel"))
    return &m_CrescendoDisplay;

  if (name == wxT("GeneralLabel"))
    return &m_BankDisplay;

  if (name == wxT("PitchLabel"))
    return m_organfile->GetPitchLabel();

  if (name == wxT("TemperamentLabel"))
    return m_organfile->GetTemperamentLabel();

  if (name == wxT("TransposeLabel"))
    return &m_TransposeDisplay;

  if (is_panel)
    return NULL;

  if (name == wxT("OrganNameLabel"))
    return &m_NameDisplay;

  return NULL;
}
