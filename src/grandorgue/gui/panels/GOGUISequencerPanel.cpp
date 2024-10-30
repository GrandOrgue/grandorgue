/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUISequencerPanel.h"

#include <wx/intl.h>

#include "combinations/GOSetter.h"
#include "primitives/go_gui_utils.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"

GOGUISequencerPanel::GOGUISequencerPanel(GOOrganController *organController)
  : m_OrganController(organController) {}

GOGUISequencerPanel::~GOGUISequencerPanel() {}

void GOGUISequencerPanel::CreatePanels(GOConfigReader &cfg) {
  m_OrganController->AddPanel(CreateSequencerPanel(cfg));
}

GOGUIPanel *GOGUISequencerPanel::CreateSequencerPanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_OrganController);
  GOGUIDisplayMetrics *metrics = new GOGUISetterDisplayMetrics(
    cfg, m_OrganController, GOGUI_SETTER_SETTER);
  panel->Init(cfg, metrics, _("Combination Setter"), wxT("SetterPanel"));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("Setter"));
  panel->AddControl(back);

  GOGUILabel *PosDisplay
    = new GOGUILabel(panel, m_OrganController->GetLabel(wxT("Label")));
  PosDisplay->Init(cfg, wxT("SetterCurrentPosition"), 325, 10);
  PosDisplay->SetFontSize(14);
  panel->AddControl(PosDisplay);

  GOGUILabel *const currFileNameDisplay = new GOGUILabel(
    panel, m_OrganController->GetLabel(GOSetter::KEY_CURR_FILE_NAME));

  currFileNameDisplay->Init(
    cfg, GOSetter::GROUP_CURR_FILE_NAME, 170, 58, wxEmptyString, 15, 380, 40);
  currFileNameDisplay->SetFontSize(14);
  panel->AddControl(currFileNameDisplay);

  unsigned curRow = 100;

  button = new GOGUIButton(
    panel,
    m_OrganController->GetButtonControl(GOSetter::KEY_REFRESH_FILES),
    false);
  button->Init(cfg, GOSetter::GROUP_REFRESH_FILES, 1, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(GOSetter::KEY_PREV_FILE), false);
  button->Init(cfg, GOSetter::GROUP_PREV_FILE, 2, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(GOSetter::KEY_NEXT_FILE), false);
  button->Init(cfg, GOSetter::GROUP_NEXT_FILE, 8, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(GOSetter::KEY_LOAD_FILE), false);
  button->Init(cfg, GOSetter::GROUP_LOAD_FILE, 9, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(GOSetter::KEY_SAVE_FILE), false);
  button->Init(cfg, GOSetter::GROUP_SAVE_FILE, 10, curRow);
  panel->AddControl(button);

  curRow++;

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Current")), false);
  button->Init(cfg, wxT("SetterCurrent"), 1, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("M100")), false);
  button->Init(cfg, wxT("SetterM100"), 2, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("M10")), false);
  button->Init(cfg, wxT("SetterM10"), 3, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("M1")), false);
  button->Init(cfg, wxT("SetterM1"), 4, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Prev")), false);
  button->Init(cfg, wxT("SetterPrev"), 5, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Next")), false);
  button->Init(cfg, wxT("SetterNext"), 6, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("P1")), false);
  button->Init(cfg, wxT("SetterP1"), 7, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("P10")), false);
  button->Init(cfg, wxT("SetterP10"), 8, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("P100")), false);
  button->Init(cfg, wxT("SetterP100"), 9, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Home")), false);
  button->Init(cfg, wxT("SetterHome"), 10, curRow);
  panel->AddControl(button);

  curRow++;

  for (unsigned i = 0; i < 10; i++) {
    button = new GOGUIButton(
      panel,
      m_OrganController->GetButtonControl(wxString::Format(wxT("L%d"), i)),
      false);
    button->Init(cfg, wxString::Format(wxT("SetterL%d"), i), i + 1, curRow);
    panel->AddControl(button);
  }

  curRow++;

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Set")), false);
  button->Init(cfg, wxT("SetterSet"), 1, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Regular")), false);
  button->Init(cfg, wxT("SetterRegular"), 3, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Scope")), false);
  button->Init(cfg, wxT("SetterScope"), 4, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Scoped")), false);
  button->Init(cfg, wxT("SetterScoped"), 5, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Full")), false);
  button->Init(cfg, wxT("SetterFull"), 7, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("GC")), false);
  button->Init(cfg, wxT("SetterGC"), 8, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Insert")), false);
  button->Init(cfg, wxT("SetterInsert"), 9, curRow);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_OrganController->GetButtonControl(wxT("Delete")), false);
  button->Init(cfg, wxT("SetterDelete"), 10, curRow);
  panel->AddControl(button);

  return panel;
}
