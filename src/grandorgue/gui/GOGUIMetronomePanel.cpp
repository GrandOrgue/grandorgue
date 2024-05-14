/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIMetronomePanel.h"

#include <wx/intl.h>

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"

GOGUIMetronomePanel::GOGUIMetronomePanel(GOOrganController *organController)
  : m_OrganController(organController) {}

GOGUIMetronomePanel::~GOGUIMetronomePanel() {}

void GOGUIMetronomePanel::CreatePanels(GOConfigReader &cfg) {
  m_OrganController->AddPanel(CreateMetronomePanel(cfg));
}

GOGUIPanel *GOGUIMetronomePanel::CreateMetronomePanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_OrganController);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_OrganController, GOGUI_METRONOME);
  panel->Init(cfg, metrics, _("Metronome"), wxT("Metronome"), wxT(""));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("Metronome"));
  panel->AddControl(back);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("MetronomeOn")), false);
  button->Init(cfg, wxT("MetronomeOn"), 1, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel,
    m_OrganController->GetButtonControl(wxT("MetronomeMeasureM1")),
    false);
  button->Init(cfg, wxT("MetronomeMM1"), 3, 100);
  panel->AddControl(button);

  GOGUILabel *PosDisplay = new GOGUILabel(
    panel, m_OrganController->GetLabel(wxT("MetronomeMeasure")));
  PosDisplay->Init(cfg, wxT("MetronomeMeasure"), 240, 45);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel,
    m_OrganController->GetButtonControl(wxT("MetronomeMeasureP1")),
    false);
  button->Init(cfg, wxT("MetronomeMP1"), 5, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("MetronomeBpmM10")), false);
  button->Init(cfg, wxT("MetronomeBPMM10"), 1, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("MetronomeBpmM1")), false);
  button->Init(cfg, wxT("MetronomeBPMM1"), 2, 101);
  panel->AddControl(button);

  PosDisplay
    = new GOGUILabel(panel, m_OrganController->GetLabel(wxT("MetronomeBPM")));
  PosDisplay->Init(cfg, wxT("MetronomeBPM"), 160, 115);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("MetronomeBpmP1")), false);
  button->Init(cfg, wxT("MetronomeBPMP1"), 4, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("MetronomeBpmP10")), false);
  button->Init(cfg, wxT("MetronomeBPMP10"), 5, 101);
  panel->AddControl(button);

  return panel;
}
