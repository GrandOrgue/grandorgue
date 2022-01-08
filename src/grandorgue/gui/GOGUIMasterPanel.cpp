/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIMasterPanel.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"

GOGUIMasterPanel::GOGUIMasterPanel(GODefinitionFile* organfile)
    : m_organfile(organfile) {}

GOGUIMasterPanel::~GOGUIMasterPanel() {}

void GOGUIMasterPanel::CreatePanels(GOConfigReader& cfg) {
  m_organfile->AddPanel(CreateMasterPanel(cfg));
}

GOGUIPanel* GOGUIMasterPanel::CreateMasterPanel(GOConfigReader& cfg) {
  GOGUIButton* button;

  GOGUIPanel* panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics* metrics =
      new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_MASTER);
  panel->Init(cfg, metrics, _("Master Controls"), wxT("SetterMaster"), wxT(""));

  GOGUIHW1Background* back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("SetterMaster"));
  panel->AddControl(back);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM100")), false);
  button->Init(cfg, wxT("SetterMasterPitchM100"), 1, 100);
  panel->AddControl(button);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM10")), false);
  button->Init(cfg, wxT("SetterMasterPitchM10"), 2, 100);
  panel->AddControl(button);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM1")), false);
  button->Init(cfg, wxT("SetterMasterPitchM1"), 3, 100);
  panel->AddControl(button);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP1")), false);
  button->Init(cfg, wxT("SetterMasterPitchP1"), 5, 100);
  panel->AddControl(button);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP10")), false);
  button->Init(cfg, wxT("SetterMasterPitchP10"), 6, 100);
  panel->AddControl(button);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP100")), false);
  button->Init(cfg, wxT("SetterMasterPitchP10"), 7, 100);
  panel->AddControl(button);

  GOGUILabel* PosDisplay =
      new GOGUILabel(panel, m_organfile->GetLabel(wxT("PitchLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterPitch"), 230, 35);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
      panel, m_organfile->GetButton(wxT("TemperamentPrev")), false);
  button->Init(cfg, wxT("SetterMasterTemperamentPrev"), 1, 101);
  panel->AddControl(button);

  PosDisplay =
      new GOGUILabel(panel, m_organfile->GetLabel(wxT("TemperamentLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterTemperament"), 80, 90, wxEmptyString,
                   2);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
      panel, m_organfile->GetButton(wxT("TemperamentNext")), false);
  button->Init(cfg, wxT("SetterMasterTemperamentNext"), 3, 101);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Save")), false);
  button->Init(cfg, wxT("SetterSave"), 5, 101);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("OnState")), true);
  button->Init(cfg, wxT("SetterOn"), 1, 100, 4);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("TransposeDown")),
                           false);
  button->Init(cfg, wxT("SetterMasterTransposeDown"), 1, 102);
  panel->AddControl(button);

  PosDisplay =
      new GOGUILabel(panel, m_organfile->GetLabel(wxT("TransposeLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterTranspose"), 80, 175);
  panel->AddControl(PosDisplay);

  button =
      new GOGUIButton(panel, m_organfile->GetButton(wxT("TransposeUp")), false);
  button->Init(cfg, wxT("SetterMasterTransposeUp"), 3, 102);
  panel->AddControl(button);

  PosDisplay =
      new GOGUILabel(panel, m_organfile->GetLabel(wxT("OrganNameLabel")));
  PosDisplay->Init(cfg, wxT("SetterMasterName"), 180, 230, wxEmptyString, 5);
  panel->AddControl(PosDisplay);

  return panel;
}
