/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUICouplerPanel.h"

#include <wx/intl.h>

#include "model/GOCoupler.h"
#include "model/GOManual.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"
#include "GOVirtualCouplerController.h"

void GOGUICouplerPanel::CreatePanels(GOConfigReader &cfg) {
  for (unsigned i = m_OrganController->GetFirstManualIndex();
       i <= m_OrganController->GetManualAndPedalCount();
       i++)
    m_OrganController->AddPanel(CreateCouplerPanel(cfg, i));
}

GOGUIPanel *GOGUICouplerPanel::CreateCouplerPanel(
  GOConfigReader &cfg, unsigned manual_nr) {
  GOManual *manual = m_OrganController->GetManual(manual_nr);

  GOGUIPanel *panel = new GOGUIPanel(m_OrganController);
  GOGUIDisplayMetrics *metrics = new GOGUISetterDisplayMetrics(
    cfg, m_OrganController, GOGUI_SETTER_COUPLER);
  panel->Init(
    cfg,
    metrics,
    wxString::Format(_("Coupler %s"), manual->GetName().c_str()),
    wxString::Format(wxT("SetterCouplers%03d"), manual_nr),
    _("Coupler"));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxString::Format(wxT("SetterCouplers%03d"), manual_nr));
  panel->AddControl(back);

  panel->GetLayoutEngine()->Update();
  for (unsigned int i = m_OrganController->GetFirstManualIndex();
       i < m_OrganController->GetODFManualCount();
       i++) {
    int x, y;
    GOManual *dest_manual = m_OrganController->GetManual(i);
    GOCoupler *coupler;
    GOButtonControl *pCouplerThough;
    GOGUIButton *button;

    panel->GetLayoutEngine()->GetDrawstopBlitPosition(100 + i, 1, x, y);

    GOGUILabel *PosDisplay = new GOGUILabel(panel, NULL);
    PosDisplay->Init(
      cfg,
      wxString::Format(wxT("SetterCoupler%03dLabel%03d"), manual_nr, i),
      x,
      y,
      dest_manual->GetName());
    panel->AddControl(PosDisplay);

    if ((coupler = r_VirtualCouplers.GetCoupler(
           manual_nr, i, GOVirtualCouplerController::COUPLER_16))) {
      button = new GOGUIButton(panel, coupler, false);
      button->Init(
        cfg,
        wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i),
        2,
        100 + i);
      panel->AddControl(button);
    }

    if ((coupler = r_VirtualCouplers.GetCoupler(
           manual_nr, i, GOVirtualCouplerController::COUPLER_8))) {
      button = new GOGUIButton(panel, coupler, false);
      button->Init(
        cfg,
        wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i),
        3,
        100 + i);
      panel->AddControl(button);
    }

    if ((coupler = r_VirtualCouplers.GetCoupler(
           manual_nr, i, GOVirtualCouplerController::COUPLER_4))) {
      button = new GOGUIButton(panel, coupler, false);
      button->Init(
        cfg,
        wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i),
        4,
        100 + i);
      panel->AddControl(button);
    }

    if ((coupler = r_VirtualCouplers.GetCoupler(
           manual_nr, i, GOVirtualCouplerController::COUPLER_BAS))) {
      button = new GOGUIButton(panel, coupler, false);
      button->Init(
        cfg,
        wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i),
        5,
        100 + i);
      panel->AddControl(button);
    }

    if ((coupler = r_VirtualCouplers.GetCoupler(
           manual_nr, i, GOVirtualCouplerController::COUPLER_MEL))) {
      button = new GOGUIButton(panel, coupler, false);
      button->Init(
        cfg,
        wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i),
        6,
        100 + i);
      panel->AddControl(button);
    }

    if ((pCouplerThough = r_VirtualCouplers.GetCouplerThrough(manual_nr, i))) {
      button = new GOGUIButton(panel, pCouplerThough, false);
      button->Init(
        cfg,
        wxString::Format(
          wxT("SetterManual%03dCoupler%03dThrough"), manual_nr, i),
        7,
        100 + i);
      panel->AddControl(button);
    }
  }

  return panel;
}
