/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

GOGUICouplerPanel::GOGUICouplerPanel(GOOrganController *organController)
  : m_OrganController(organController) {}

GOGUICouplerPanel::~GOGUICouplerPanel() {}

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

    coupler = new GOCoupler(*m_OrganController, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i),
      _("16"),
      false,
      false,
      -12,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC16"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i),
      2,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(*m_OrganController, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i),
      manual_nr != i ? _("8") : _("U.O."),
      manual_nr == i,
      false,
      0,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC8"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i),
      3,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(*m_OrganController, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i),
      _("4"),
      false,
      false,
      12,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC4"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i),
      4,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(*m_OrganController, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i),
      _("BAS"),
      false,
      false,
      0,
      i,
      GOCoupler::COUPLER_BASS);
    coupler->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dCB"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i),
      5,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(*m_OrganController, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i),
      _("MEL"),
      false,
      false,
      0,
      i,
      GOCoupler::COUPLER_MELODY);
    coupler->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dCM"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i),
      6,
      100 + i);
    panel->AddControl(button);
  }

  return panel;
}
