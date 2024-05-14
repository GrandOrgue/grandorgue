/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIFloatingPanel.h"

#include <wx/intl.h>

#include "combinations/GODivisionalSetter.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "model/GOWindchest.h"

#include "GOGUIButton.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"

GOGUIFloatingPanel::GOGUIFloatingPanel(GOOrganController *organController)
  : m_OrganController(organController) {}

GOGUIFloatingPanel::~GOGUIFloatingPanel() {}

void GOGUIFloatingPanel::CreatePanels(GOConfigReader &cfg) {
  m_OrganController->AddPanel(CreateFloatingPanel(cfg));
}

GOGUIPanel *GOGUIFloatingPanel::CreateFloatingPanel(GOConfigReader &cfg) {
  GOGUIPanel *panel = new GOGUIPanel(m_OrganController);
  GOGUIDisplayMetrics *metrics = new GOGUISetterDisplayMetrics(
    cfg, m_OrganController, GOGUI_SETTER_FLOATING);
  panel->Init(
    cfg,
    metrics,
    _("Coupler manuals and volume"),
    wxT("SetterFloating"),
    wxT(""));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("SetterFloating"));
  panel->AddControl(back);

  for (unsigned i = m_OrganController->GetODFManualCount();
       i <= m_OrganController->GetManualAndPedalCount();
       i++) {
    wxString group;
    group.Printf(
      wxT("SetterFloating%03d"),
      i - m_OrganController->GetODFManualCount() + 1);
    GOGUIManualBackground *manual_back = new GOGUIManualBackground(
      panel, i - m_OrganController->GetODFManualCount());
    manual_back->Init(cfg, group);
    panel->AddControl(manual_back);

    GOGUIManual *manual = new GOGUIManual(
      panel,
      m_OrganController->GetManual(i),
      i - m_OrganController->GetODFManualCount());
    manual->Init(cfg, group);
    panel->AddControl(manual);

    /*
    for (unsigned j = 0; j < 10; j++) {
      wxString buttonName = GODivisionalSetter::GetDivisionalButtonName(i, j);
      GOButtonControl *const divisional
        = m_OrganController->GetButtonControl(buttonName, false);

      GOGUIButton *button = new GOGUIButton(panel, divisional, true);
      button->Init(
        cfg, buttonName, j + 1, i - m_OrganController->GetODFManualCount());
      panel->AddControl(button);
    }
    */
  }

  GOEnclosure *master_enc = new GOEnclosure(*m_OrganController);
  master_enc->Init(cfg, wxT("SetterMasterVolume"), _("Master"), 127);
  m_OrganController->AddEnclosure(master_enc);
  master_enc->SetElementID(
    m_OrganController->GetRecorderElementID(wxString::Format(wxT("SM"))));

  GOGUIEnclosure *enclosure = new GOGUIEnclosure(panel, master_enc);
  enclosure->Init(cfg, wxT("SetterMasterVolume"));
  panel->AddControl(enclosure);

  for (unsigned i = 0; i < m_OrganController->GetWindchestCount(); i++) {
    GOWindchest *windchest = m_OrganController->GetWindchest(i);
    windchest->AddEnclosure(master_enc);

    GOEnclosure *enc = new GOEnclosure(*m_OrganController);
    enc->Init(
      cfg,
      wxString::Format(wxT("SetterMaster%03d"), i + 1),
      windchest->GetName(),
      127);
    m_OrganController->AddEnclosure(enc);
    enc->SetElementID(m_OrganController->GetRecorderElementID(
      wxString::Format(wxT("SM%d"), i)));
    windchest->AddEnclosure(enc);

    enclosure = new GOGUIEnclosure(panel, enc);
    enclosure->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1));
    panel->AddControl(enclosure);
  }

  return panel;
}
