/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUICouplerManualsAndVolumePanel.h"

#include <wx/intl.h>

#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOOrganController.h"
#include "gui/GOGuiOrgan.h"

GOGUICouplerManualsAndVolumePanel::GOGUICouplerManualsAndVolumePanel(
  GOGuiOrgan &guiOrgan)
  : r_GuiOrgan(guiOrgan), m_OrganController(guiOrgan.GetOrganController()) {}

GOGUICouplerManualsAndVolumePanel::~GOGUICouplerManualsAndVolumePanel() {}

void GOGUICouplerManualsAndVolumePanel::CreatePanels(GOConfigReader &cfg) {
  r_GuiOrgan.AddPanel(CreatePanel(cfg));
}

GOGUIPanel *GOGUICouplerManualsAndVolumePanel::CreatePanel(
  GOConfigReader &cfg) {
  GOGUIPanel *panel = new GOGUIPanel(
    m_OrganController, r_GuiOrgan.GetImageCache(), r_GuiOrgan.GetMouseState());
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
  }

  GOGUIEnclosure *enclosure
    = new GOGUIEnclosure(panel, m_OrganController->GetMasterEnclosure());

  enclosure->Init(cfg, wxT("SetterMasterVolume"));
  panel->AddControl(enclosure);

  for (unsigned nWindchests = m_OrganController->GetWindchestCount(),
                windchestI = 0;
       windchestI < nWindchests;
       windchestI++) {
    enclosure = new GOGUIEnclosure(
      panel, m_OrganController->GetInternalEnclosure(windchestI));
    enclosure->Init(
      cfg, wxString::Format(wxT("SetterMaster%03d"), windchestI + 1));
    panel->AddControl(enclosure);
  }
  return panel;
}
