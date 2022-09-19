/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUISequencerPanel.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"

GOGUISequencerPanel::GOGUISequencerPanel(GODefinitionFile *organfile)
  : m_organfile(organfile) {}

GOGUISequencerPanel::~GOGUISequencerPanel() {}

void GOGUISequencerPanel::CreatePanels(GOConfigReader &cfg) {
  m_organfile->AddPanel(CreateSequencerPanel(cfg));
}

GOGUIPanel *GOGUISequencerPanel::CreateSequencerPanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_SETTER);
  panel->Init(cfg, metrics, _("Combination Setter"), wxT("SetterPanel"));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("Setter"));
  panel->AddControl(back);

  GOGUILabel *PosDisplay
    = new GOGUILabel(panel, m_organfile->GetLabel(wxT("Label")));
  PosDisplay->Init(cfg, wxT("SetterCurrentPosition"), 350, 10);
  panel->AddControl(PosDisplay);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Current")), false);
  button->Init(cfg, wxT("SetterCurrent"), 1, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("M100")), false);
  button->Init(cfg, wxT("SetterM100"), 2, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("M10")), false);
  button->Init(cfg, wxT("SetterM10"), 3, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("M1")), false);
  button->Init(cfg, wxT("SetterM1"), 4, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Prev")), false);
  button->Init(cfg, wxT("SetterPrev"), 5, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Next")), false);
  button->Init(cfg, wxT("SetterNext"), 6, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("P1")), false);
  button->Init(cfg, wxT("SetterP1"), 7, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("P10")), false);
  button->Init(cfg, wxT("SetterP10"), 8, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("P100")), false);
  button->Init(cfg, wxT("SetterP100"), 9, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Home")), false);
  button->Init(cfg, wxT("SetterHome"), 10, 100);
  panel->AddControl(button);

  for (unsigned i = 0; i < 10; i++) {
    button = new GOGUIButton(
      panel,
      m_organfile->GetButtonControl(wxString::Format(wxT("L%d"), i)),
      false);
    button->Init(cfg, wxString::Format(wxT("SetterL%d"), i), i + 1, 101);
    panel->AddControl(button);
  }

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Set")), false);
  button->Init(cfg, wxT("SetterSet"), 1, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Regular")), false);
  button->Init(cfg, wxT("SetterRegular"), 3, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Scope")), false);
  button->Init(cfg, wxT("SetterScope"), 4, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Scoped")), false);
  button->Init(cfg, wxT("SetterScoped"), 5, 102);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Full")), false);
  button->Init(cfg, wxT("SetterFull"), 7, 102);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("GC")), false);
  button->Init(cfg, wxT("SetterGC"), 8, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Insert")), false);
  button->Init(cfg, wxT("SetterInsert"), 9, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Delete")), false);
  button->Init(cfg, wxT("SetterDelete"), 10, 102);
  panel->AddControl(button);

  return panel;
}
