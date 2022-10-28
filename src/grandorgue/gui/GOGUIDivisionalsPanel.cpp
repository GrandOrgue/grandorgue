/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIDivisionalsPanel.h"

#include <wx/intl.h>

// #include "combinations/control/GODivisionalButtonControl.h"
#include "combinations/GODivisionalSetter.h"
#include "control/GOButtonControl.h"

#include "GODefinitionFile.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOManual.h"

GOGUIDivisionalsPanel::GOGUIDivisionalsPanel(GODefinitionFile *organfile)
  : m_organfile(organfile) {}

GOGUIDivisionalsPanel::~GOGUIDivisionalsPanel() {}

void GOGUIDivisionalsPanel::CreatePanels(GOConfigReader &cfg) {
  m_organfile->AddPanel(CreateDivisionalsPanel(cfg));
}

GOGUIPanel *GOGUIDivisionalsPanel::CreateDivisionalsPanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_DIVISIONALS);
  panel->Init(cfg, metrics, _("Divisionals"), wxT("SetterDivisionalPanel"));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("SetterDivisionals"));
  panel->AddControl(back);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Set")), false);
  button->Init(cfg, wxT("SetterGeneralsSet"), 1, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Regular")), false);
  button->Init(cfg, wxT("SetterGerneralsRegular"), 3, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Scope")), false);
  button->Init(cfg, wxT("SetterGeneralsScope"), 4, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButtonControl(wxT("Scoped")), false);
  button->Init(cfg, wxT("SetterGeneralsScoped"), 5, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButtonControl(wxT("Full")), false);
  button->Init(cfg, wxT("SetterGeneralsFull"), 7, 100);
  panel->AddControl(button);

  GOGUILayoutEngine *const pLayout = panel->GetLayoutEngine();

  pLayout->Update();
  for (unsigned int i = m_organfile->GetFirstManualIndex();
       i < m_organfile->GetODFManualCount();
       i++) {
    int x, y;
    GOManual *manual = m_organfile->GetManual(i);

    pLayout->GetPushbuttonBlitPosition(100 + i, 1, x, y);

    GOGUILabel *PosDisplay = new GOGUILabel(panel, NULL);
    PosDisplay->Init(
      cfg,
      wxString::Format(wxT("SetterDivisionalLabel%03d"), i),
      x,
      y,
      manual->GetName());
    panel->AddControl(PosDisplay);

    for (unsigned j = 0; j < 10; j++) {
      wxString buttonName = GODivisionalSetter::GetDivisionalButtonName(i, j);
      GOButtonControl *const divisional
        = m_organfile->GetButtonControl(buttonName, false);

      // manual->AddDivisional(divisional);

      button = new GOGUIButton(panel, divisional, true);
      button->Init(cfg, buttonName, j + 3, 100 + i);
      panel->AddControl(button);
    }

    wxString labelName = GODivisionalSetter::GetDivisionalBankLabelName(i);
    GOGUILabel *bankDisplay
      = new GOGUILabel(panel, m_organfile->GetLabel(labelName));

    pLayout->GetPushbuttonBlitPosition(100 + i, 13, x, y);
    bankDisplay->Init(cfg, labelName, x, y);
    panel->AddControl(bankDisplay);

    wxString divisionalPrevName
      = GODivisionalSetter::GetDivisionalBankPrevLabelName(i);
    GOButtonControl *const divisionalPrev
      = m_organfile->GetButtonControl(divisionalPrevName, false);

    button = new GOGUIButton(panel, divisionalPrev, true);
    button->Init(cfg, divisionalPrevName, 15, 100 + i);
    panel->AddControl(button);

    wxString divisionalNextName
      = GODivisionalSetter::GetDivisionalBankNextLabelName(i);
    GOButtonControl *const divisionalNext
      = m_organfile->GetButtonControl(divisionalNextName, false);

    button = new GOGUIButton(panel, divisionalNext, true);
    button->Init(cfg, divisionalNextName, 16, 100 + i);
    panel->AddControl(button);
  }
  return panel;
}
