/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUICouplerPanel.h"

#include <wx/intl.h>

#include "model/GOManual.h"

#include "GOCoupler.h"
#include "GODefinitionFile.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"

GOGUICouplerPanel::GOGUICouplerPanel(GODefinitionFile *organfile)
  : m_organfile(organfile) {}

GOGUICouplerPanel::~GOGUICouplerPanel() {}

void GOGUICouplerPanel::CreatePanels(GOConfigReader &cfg) {
  for (unsigned i = m_organfile->GetFirstManualIndex();
       i <= m_organfile->GetManualAndPedalCount();
       i++)
    m_organfile->AddPanel(CreateCouplerPanel(cfg, i));
}

GOGUIPanel *GOGUICouplerPanel::CreateCouplerPanel(
  GOConfigReader &cfg, unsigned manual_nr) {
  GOManual *manual = m_organfile->GetManual(manual_nr);

  GOGUIPanel *panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_COUPLER);
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
  for (unsigned int i = m_organfile->GetFirstManualIndex();
       i < m_organfile->GetODFManualCount();
       i++) {
    int x, y;
    GOManual *dest_manual = m_organfile->GetManual(i);
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

    coupler = new GOCoupler(m_organfile, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i),
      _("16"),
      false,
      false,
      -12,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_organfile->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC16"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i),
      2,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(m_organfile, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i),
      manual_nr != i ? _("8") : _("U.O."),
      manual_nr == i,
      false,
      0,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_organfile->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC8"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i),
      3,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(m_organfile, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i),
      _("4"),
      false,
      false,
      12,
      i,
      GOCoupler::COUPLER_NORMAL);
    coupler->SetElementID(m_organfile->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dC4"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i),
      4,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(m_organfile, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i),
      _("BAS"),
      false,
      false,
      0,
      i,
      GOCoupler::COUPLER_BASS);
    coupler->SetElementID(m_organfile->GetRecorderElementID(
      wxString::Format(wxT("S%dM%dCB"), manual_nr, i)));
    manual->AddCoupler(coupler);
    button = new GOGUIButton(panel, coupler, false);
    button->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i),
      5,
      100 + i);
    panel->AddControl(button);

    coupler = new GOCoupler(m_organfile, manual_nr);
    coupler->Init(
      cfg,
      wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i),
      _("MEL"),
      false,
      false,
      0,
      i,
      GOCoupler::COUPLER_MELODY);
    coupler->SetElementID(m_organfile->GetRecorderElementID(
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
