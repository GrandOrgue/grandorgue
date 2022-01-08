/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLabel.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GODocument.h"
#include "config/GOConfig.h"

GOLabel::GOLabel(GODefinitionFile *organfile)
    : m_Name(), m_Content(), m_organfile(organfile),
      m_sender(organfile, MIDI_SEND_LABEL) {
  m_organfile->RegisterMidiConfigurator(this);
  m_organfile->RegisterPlaybackStateHandler(this);
}

GOLabel::~GOLabel() {}

void GOLabel::Init(GOConfigReader &cfg, wxString group, wxString name) {
  m_organfile->RegisterSaveableObject(this);
  m_group = group;
  m_Name = name;
  m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOLabel::Load(GOConfigReader &cfg, wxString group, wxString name) {
  m_organfile->RegisterSaveableObject(this);
  m_group = group;
  m_Name = name;
  m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOLabel::Save(GOConfigWriter &cfg) {
  m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

const wxString &GOLabel::GetName() { return m_Name; }

const wxString &GOLabel::GetContent() { return m_Content; }

void GOLabel::SetContent(wxString name) {
  m_Content = name;
  m_sender.SetLabel(m_Content);
  m_organfile->ControlChanged(this);
}

void GOLabel::AbortPlayback() {
  m_sender.SetLabel(wxEmptyString);
  m_sender.SetName(wxEmptyString);
}

void GOLabel::PreparePlayback() { m_sender.SetName(m_Name); }

void GOLabel::StartPlayback() {}

void GOLabel::PrepareRecording() { m_sender.SetLabel(m_Content); }

wxString GOLabel::GetMidiType() { return _("Label"); }

wxString GOLabel::GetMidiName() { return GetName(); }

void GOLabel::ShowConfigDialog() {
  wxString title = wxString::Format(
    _("Midi-Settings for %s - %s"),
    GetMidiType().c_str(),
    GetMidiName().c_str());

  m_organfile->GetDocument()->ShowMIDIEventDialog(
    this, title, NULL, &m_sender, NULL);
}

wxString GOLabel::GetElementStatus() { return m_Content; }

std::vector<wxString> GOLabel::GetElementActions() {
  std::vector<wxString> actions;
  return actions;
}

void GOLabel::TriggerElementActions(unsigned no) {
  // Never called
}
