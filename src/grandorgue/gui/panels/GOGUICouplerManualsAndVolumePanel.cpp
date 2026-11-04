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
  : r_GuiOrgan(guiOrgan), p_OrganController(guiOrgan.GetOrganController()) {}

GOGUICouplerManualsAndVolumePanel::~GOGUICouplerManualsAndVolumePanel() {}

void GOGUICouplerManualsAndVolumePanel::CreatePanels(GOConfigReader &cfg) {
  r_GuiOrgan.AddPanel(CreatePanel(cfg));
}

GOGUIPanel *GOGUICouplerManualsAndVolumePanel::CreatePanel(
  GOConfigReader &cfg) {
  GOGUIPanel *pPanel = new GOGUIPanel(
    p_OrganController, r_GuiOrgan.GetImageCache(), r_GuiOrgan.GetMouseState());
  GOGUIDisplayMetrics *pMetrics = new GOGUISetterDisplayMetrics(
    cfg, p_OrganController, GOGUI_COUPLER_MANUALS_AND_VOLUME);

  pPanel->Init(
    cfg,
    pMetrics,
    _("Coupler manuals and volume"),
    // The .cmb group is still named SetterFloating for compatibility
    // with the existing organ settings
    wxT("SetterFloating"),
    wxT(""));

  GOGUIHW1Background *pBack = new GOGUIHW1Background(pPanel);
  pBack->Init(cfg, wxT("SetterFloating"));
  pPanel->AddControl(pBack);

  for (unsigned i = p_OrganController->GetODFManualCount();
       i <= p_OrganController->GetManualAndPedalCount();
       i++) {
    wxString group;
    group.Printf(
      wxT("SetterFloating%03d"),
      i - p_OrganController->GetODFManualCount() + 1);
    GOGUIManualBackground *pManualBack = new GOGUIManualBackground(
      pPanel, i - p_OrganController->GetODFManualCount());
    pManualBack->Init(cfg, group);
    pPanel->AddControl(pManualBack);

    GOGUIManual *pManual = new GOGUIManual(
      pPanel,
      p_OrganController->GetManual(i),
      i - p_OrganController->GetODFManualCount());
    pManual->Init(cfg, group);
    pPanel->AddControl(pManual);
  }

  GOGUIEnclosure *pEnclosure
    = new GOGUIEnclosure(pPanel, p_OrganController->GetMasterEnclosure());

  pEnclosure->Init(cfg, wxT("SetterMasterVolume"));
  pPanel->AddControl(pEnclosure);

  for (unsigned nWindchests = p_OrganController->GetWindchestCount(),
                windchestI = 0;
       windchestI < nWindchests;
       windchestI++) {
    pEnclosure = new GOGUIEnclosure(
      pPanel, p_OrganController->GetInternalEnclosure(windchestI));
    pEnclosure->Init(
      cfg, wxString::Format(wxT("SetterMaster%03d"), windchestI + 1));
    pPanel->AddControl(pEnclosure);
  }
  return pPanel;
}
