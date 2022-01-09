/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIFloatingPanel.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GODivisional.h"
#include "GOEnclosure.h"
#include "GOGUIButton.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOManual.h"
#include "GOWindchest.h"

GOGUIFloatingPanel::GOGUIFloatingPanel(GODefinitionFile *organfile)
  : m_organfile(organfile) {}

GOGUIFloatingPanel::~GOGUIFloatingPanel() {}

void GOGUIFloatingPanel::CreatePanels(GOConfigReader &cfg) {
  m_organfile->AddPanel(CreateFloatingPanel(cfg));
}

GOGUIPanel *GOGUIFloatingPanel::CreateFloatingPanel(GOConfigReader &cfg) {
  GOGUIPanel *panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_FLOATING);
  panel->Init(
    cfg,
    metrics,
    _("Coupler manuals and volume"),
    wxT("SetterFloating"),
    wxT(""));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("SetterFloating"));
  panel->AddControl(back);

  for (unsigned i = m_organfile->GetODFManualCount();
       i <= m_organfile->GetManualAndPedalCount();
       i++) {
    wxString group;
    group.Printf(
      wxT("SetterFloating%03d"), i - m_organfile->GetODFManualCount() + 1);
    GOGUIManualBackground *manual_back
      = new GOGUIManualBackground(panel, i - m_organfile->GetODFManualCount());
    manual_back->Init(cfg, group);
    panel->AddControl(manual_back);

    GOGUIManual *manual = new GOGUIManual(
      panel, m_organfile->GetManual(i), i - m_organfile->GetODFManualCount());
    manual->Init(cfg, group);
    panel->AddControl(manual);

    for (unsigned j = 0; j < 10; j++) {
      GODivisional *divisional = new GODivisional(
        m_organfile, m_organfile->GetManual(i)->GetDivisionalTemplate(), true);
      divisional->Init(
        cfg,
        wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100),
        i,
        100 + j,
        wxString::Format(wxT("%d"), j + 1));
      m_organfile->GetManual(i)->AddDivisional(divisional);

      GOGUIButton *button = new GOGUIButton(panel, divisional, true);
      button->Init(
        cfg,
        wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100),
        j + 1,
        i - m_organfile->GetODFManualCount());
      panel->AddControl(button);
    }
  }

  GOEnclosure *master_enc = new GOEnclosure(m_organfile);
  master_enc->Init(cfg, wxT("SetterMasterVolume"), _("Master"), 127);
  m_organfile->AddEnclosure(master_enc);
  master_enc->SetElementID(
    m_organfile->GetRecorderElementID(wxString::Format(wxT("SM"))));

  GOGUIEnclosure *enclosure = new GOGUIEnclosure(panel, master_enc);
  enclosure->Init(cfg, wxT("SetterMasterVolume"));
  panel->AddControl(enclosure);

  for (unsigned i = 0; i < m_organfile->GetWindchestGroupCount(); i++) {
    GOWindchest *windchest = m_organfile->GetWindchest(i);
    windchest->AddEnclosure(master_enc);

    GOEnclosure *enc = new GOEnclosure(m_organfile);
    enc->Init(
      cfg,
      wxString::Format(wxT("SetterMaster%03d"), i + 1),
      windchest->GetName(),
      127);
    m_organfile->AddEnclosure(enc);
    enc->SetElementID(
      m_organfile->GetRecorderElementID(wxString::Format(wxT("SM%d"), i)));
    windchest->AddEnclosure(enc);

    enclosure = new GOGUIEnclosure(panel, enc);
    enclosure->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1));
    panel->AddControl(enclosure);
  }

  return panel;
}
