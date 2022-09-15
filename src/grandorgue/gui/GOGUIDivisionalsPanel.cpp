/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIDivisionalsPanel.h"

#include <wx/intl.h>

#include "combinations/control/GODivisionalButtonControl.h"

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

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Set")), false);
  button->Init(cfg, wxT("SetterGeneralsSet"), 1, 100);
  panel->AddControl(button);

  button
    = new GOGUIButton(panel, m_organfile->GetButton(wxT("Regular")), false);
  button->Init(cfg, wxT("SetterGerneralsRegular"), 3, 100);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scope")), false);
  button->Init(cfg, wxT("SetterGeneralsScope"), 4, 100);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scoped")), false);
  button->Init(cfg, wxT("SetterGeneralsScoped"), 5, 100);
  panel->AddControl(button);

  button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Full")), false);
  button->Init(cfg, wxT("SetterGeneralsFull"), 7, 100);
  panel->AddControl(button);

  panel->GetLayoutEngine()->Update();
  for (unsigned int i = m_organfile->GetFirstManualIndex();
       i < m_organfile->GetODFManualCount();
       i++) {
    int x, y;
    GOManual *manual = m_organfile->GetManual(i);

    panel->GetLayoutEngine()->GetPushbuttonBlitPosition(100 + i, 1, x, y);

    GOGUILabel *PosDisplay = new GOGUILabel(panel, NULL);
    PosDisplay->Init(
      cfg,
      wxString::Format(wxT("SetterDivisionalLabel%03d"), i),
      x,
      y,
      manual->GetName());
    panel->AddControl(PosDisplay);

    for (unsigned j = 0; j < 10; j++) {
      GODivisionalButtonControl *divisional = new GODivisionalButtonControl(
        m_organfile, manual->GetDivisionalTemplate(), true);
      divisional->Init(
        cfg,
        wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100),
        i,
        100 + j,
        wxString::Format(wxT("%d"), j + 1));
      manual->AddDivisional(divisional);

      button = new GOGUIButton(panel, divisional, true);
      button->Init(
        cfg,
        wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100),
        j + 3,
        100 + i);
      panel->AddControl(button);
    }
  }
  return panel;
}
