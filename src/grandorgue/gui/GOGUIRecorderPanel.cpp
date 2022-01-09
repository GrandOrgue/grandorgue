/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIRecorderPanel.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"

GOGUIRecorderPanel::GOGUIRecorderPanel(GODefinitionFile *organfile)
  : m_organfile(organfile) {}

GOGUIRecorderPanel::~GOGUIRecorderPanel() {}

void GOGUIRecorderPanel::CreatePanels(GOConfigReader &cfg) {
  m_organfile->AddPanel(CreateRecorderPanel(cfg));
}

GOGUIPanel *GOGUIRecorderPanel::CreateRecorderPanel(GOConfigReader &cfg) {
  GOGUIButton *button;

  GOGUIPanel *panel = new GOGUIPanel(m_organfile);
  GOGUIDisplayMetrics *metrics
    = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_RECORDER);
  panel->Init(cfg, metrics, _("Recorder / Player"), wxT("Recorder"), wxT(""));

  GOGUIHW1Background *back = new GOGUIHW1Background(panel);
  back->Init(cfg, wxT("Recorder"));
  panel->AddControl(back);

  GOGUILabel *label = new GOGUILabel(panel, NULL);
  label->Init(cfg, wxT("MidiRecorderNameLabel"), 1, 45, _("Audio Recorder"));
  panel->AddControl(label);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("AudioRecorderRecord")), false);
  button->Init(cfg, wxT("MidiRecorderRecord"), 2, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("AudioRecorderStop")), false);
  button->Init(cfg, wxT("MidiRecorderStop"), 3, 100);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("AudioRecorderRecordRename")), false);
  button->Init(cfg, wxT("MidiRecorderRecordRename"), 4, 100);
  panel->AddControl(button);

  label
    = new GOGUILabel(panel, m_organfile->GetLabel(wxT("AudioRecorderLabel")));
  label->Init(cfg, wxT("MidiRecorderLabel"), 310, 45, wxT(""));
  panel->AddControl(label);

  label = new GOGUILabel(panel, NULL);
  label->Init(cfg, wxT("MidiRecorderNameLabel"), 1, 115, _("MIDI Recorder"));
  panel->AddControl(label);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiRecorderRecord")), false);
  button->Init(cfg, wxT("MidiRecorderRecord"), 2, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiRecorderStop")), false);
  button->Init(cfg, wxT("MidiRecorderStop"), 3, 101);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiRecorderRecordRename")), false);
  button->Init(cfg, wxT("MidiRecorderRecordRename"), 4, 101);
  panel->AddControl(button);

  label
    = new GOGUILabel(panel, m_organfile->GetLabel(wxT("MidiRecorderLabel")));
  label->Init(cfg, wxT("MidiRecorderLabel"), 310, 115, wxT(""));
  panel->AddControl(label);

  label = new GOGUILabel(panel, NULL);
  label->Init(cfg, wxT("MidiRecorderNameLabel"), 1, 185, _("MIDI Player"));
  panel->AddControl(label);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiPlayerPlay")), false);
  button->Init(cfg, wxT("MidiPlayerPlay"), 2, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiPlayerStop")), false);
  button->Init(cfg, wxT("MidiPlayerStop"), 3, 102);
  panel->AddControl(button);

  button = new GOGUIButton(
    panel, m_organfile->GetButton(wxT("MidiPlayerPause")), false);
  button->Init(cfg, wxT("MidiPlayerPause"), 4, 102);
  panel->AddControl(button);

  label = new GOGUILabel(panel, m_organfile->GetLabel(wxT("MidiPlayerLabel")));
  label->Init(cfg, wxT("MidiPlayerLabel"), 310, 185, wxT(""));
  panel->AddControl(label);

  return panel;
}
