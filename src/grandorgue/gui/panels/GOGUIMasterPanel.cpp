/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIMasterPanel.h"

#include <wx/intl.h>

#include "combinations/GOSetter.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"

GOGUIMasterPanel::GOGUIMasterPanel(GOOrganController *organController)
  : m_OrganController(organController) {}

GOGUIMasterPanel::~GOGUIMasterPanel() {}

void GOGUIMasterPanel::CreatePanels(GOConfigReader &cfg) {
  m_OrganController->AddPanel(CreateMasterPanel(cfg));
}

GOGUIPanel *GOGUIMasterPanel::CreateMasterPanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_OrganController);
  GOGUIDisplayMetrics *metrics = new GOGUISetterDisplayMetrics(
    cfg, m_OrganController, GOGUI_SETTER_MASTER);
  panel->Init(cfg, metrics, _("Master Controls"), wxT("SetterMaster"), wxT(""));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("SetterMaster"));
  panel->AddControl(back);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchM100")), false);
  button->Init(cfg, wxT("SetterMasterPitchM100"), 1, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchM10")), false);
  button->Init(cfg, wxT("SetterMasterPitchM10"), 2, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchM1")), false);
  button->Init(cfg, wxT("SetterMasterPitchM1"), 3, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchP1")), false);
  button->Init(cfg, wxT("SetterMasterPitchP1"), 5, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchP10")), false);
  button->Init(cfg, wxT("SetterMasterPitchP10"), 6, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("PitchP100")), false);
  button->Init(cfg, wxT("SetterMasterPitchP10"), 7, 100);
  panel->AddControl(button);

  GOGUILabel *PosDisplay
    = new GOGUILabel(panel, m_OrganController->GetLabel(wxT("PitchLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterPitch"), 230, 35);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("TemperamentPrev")), false);
  button->Init(cfg, wxT("SetterMasterTemperamentPrev"), 1, 101);
  panel->AddControl(button);

  PosDisplay = new GOGUILabel(
    panel, m_OrganController->GetLabel(wxT("TemperamentLabel")));
  PosDisplay->Init(
    cfg, wxT("SetterMasterTemperament"), 80, 90, wxEmptyString, 2);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("TemperamentNext")), false);
  button->Init(cfg, wxT("SetterMasterTemperamentNext"), 3, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Save")), false);
  button->Init(cfg, wxT("SetterSave"), 5, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(GOSetter::KEY_ON_STATE), true);
  button->Init(cfg, wxT("SetterOn"), 1, 100, 4);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("TransposeDown")), false);
  button->Init(cfg, wxT("SetterMasterTransposeDown"), 1, 102);
  panel->AddControl(button);

  PosDisplay
    = new GOGUILabel(panel, m_OrganController->GetLabel(wxT("TransposeLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterTranspose"), 80, 175);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("TransposeUp")), false);
  button->Init(cfg, wxT("SetterMasterTransposeUp"), 3, 102);
  panel->AddControl(button);

  PosDisplay
    = new GOGUILabel(panel, m_OrganController->GetLabel(wxT("OrganNameLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterName"), 180, 230, wxEmptyString, 5);
  panel->AddControl(PosDisplay);

  return panel;
}
