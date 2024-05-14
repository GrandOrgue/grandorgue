/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPlayer.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "control/GOCallbackButtonControl.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiFileReader.h"
#include "midi/GOMidiMap.h"

#include "GOEvent.h"
#include "GOOrganController.h"

enum {
  ID_MIDI_PLAYER_PLAY = 0,
  ID_MIDI_PLAYER_STOP,
  ID_MIDI_PLAYER_PAUSE,
};

const struct GOElementCreator::ButtonDefinitionEntry
  GOMidiPlayer::m_element_types[]
  = {
    {wxT("MidiPlayerPlay"), ID_MIDI_PLAYER_PLAY, false, true, false},
    {wxT("MidiPlayerStop"), ID_MIDI_PLAYER_STOP, false, true, false},
    {wxT("MidiPlayerPause"), ID_MIDI_PLAYER_PAUSE, false, true, false},
    {wxT(""), -1, false, false, false},
};

const struct GOElementCreator::ButtonDefinitionEntry *GOMidiPlayer::
  GetButtonDefinitionList() {
  return m_element_types;
}

GOMidiPlayer::GOMidiPlayer(GOOrganController *organController)
  : m_OrganController(organController),
    m_content(),
    m_PlayingTime(organController),
    m_Start(0),
    m_PlayingSeconds(0),
    m_Speed(1),
    m_IsPlaying(false),
    m_Pause(false) {
  CreateButtons(*m_OrganController);
  Clear();
  m_DeviceID
    = m_OrganController->GetSettings().GetMidiMap().GetDeviceIdByLogicalName(
      _("GrandOrgue MIDI Player"));
  UpdateDisplay();
}

GOMidiPlayer::~GOMidiPlayer() { StopPlaying(); }

void GOMidiPlayer::Load(GOConfigReader &cfg) {
  m_buttons[ID_MIDI_PLAYER_PLAY]->Init(cfg, wxT("MidiPlayerPlay"), _("PLAY"));
  m_buttons[ID_MIDI_PLAYER_STOP]->Init(cfg, wxT("MidiPlayerStop"), _("STOP"));
  m_buttons[ID_MIDI_PLAYER_PAUSE]->Init(
    cfg, wxT("MidiPlayerPause"), _("PAUSE"));
  m_PlayingTime.Init(cfg, wxT("MidiPlayerTime"), _("MIDI playing time"));
}

void GOMidiPlayer::ButtonStateChanged(int id, bool newState) {
  switch (id) {
  case ID_MIDI_PLAYER_STOP:
    StopPlaying();
    break;

  case ID_MIDI_PLAYER_PLAY:
    Play();
    break;

  case ID_MIDI_PLAYER_PAUSE:
    Pause();
    break;
  }
}

void GOMidiPlayer::Clear() {
  StopPlaying();
  m_content.Clear();
}

void GOMidiPlayer::LoadFile(
  const wxString &filename, unsigned manuals, bool pedal) {
  Clear();
  GOMidiFileReader reader(m_OrganController->GetSettings().GetMidiMap());
  if (!reader.Open(filename)) {
    GOMessageBox(
      wxString::Format(_("Failed to load %s"), filename.c_str()),
      _("MIDI Player"),
      wxOK | wxICON_ERROR,
      NULL);
    return;
  }
  if (!m_content.Load(
        reader,
        m_OrganController->GetSettings().GetMidiMap(),
        manuals,
        pedal)) {
    m_content.Clear();
    GOMessageBox(
      wxString::Format(_("Failed to load %s"), filename.c_str()),
      _("MIDI Player"),
      wxOK | wxICON_ERROR,
      NULL);
    return;
  }
  if (!reader.Close()) {
    GOMessageBox(
      wxString::Format(_("Failed to decode %s"), filename.c_str()),
      _("MIDI Player"),
      wxOK | wxICON_ERROR,
      NULL);
    return;
  }
}

bool GOMidiPlayer::IsLoaded() { return m_content.IsLoaded(); }

void GOMidiPlayer::Play() {
  StopPlaying();
  m_content.Reset();
  m_Start = wxGetLocalTimeMillis();
  m_PlayingSeconds = 0;
  m_IsPlaying = IsLoaded();
  m_Pause = false;
  if (m_IsPlaying) {
    m_buttons[ID_MIDI_PLAYER_PLAY]->Display(true);
    UpdateDisplay();
    HandleTimer();
  } else
    StopPlaying();
}

void GOMidiPlayer::Pause() {
  if (!m_IsPlaying)
    return;
  if (m_Pause) {
    m_Pause = false;
    m_buttons[ID_MIDI_PLAYER_PAUSE]->Display(m_Pause);
    m_Start = wxGetLocalTimeMillis() - m_Start;
    HandleTimer();
  } else {
    m_Pause = true;
    m_buttons[ID_MIDI_PLAYER_PAUSE]->Display(m_Pause);
    m_Start = wxGetLocalTimeMillis() - m_Start;
    m_OrganController->DeleteTimer(this);
  }
}

void GOMidiPlayer::StopPlaying() {
  if (m_IsPlaying) {
    for (unsigned i = 1; i < 16; i++) {
      GOMidiEvent e;
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(i);
      e.SetKey(MIDI_CTRL_NOTES_OFF);
      e.SetValue(0);
      e.SetDevice(m_DeviceID);
      e.SetTime(wxGetLocalTimeMillis());
      m_OrganController->ProcessMidi(e);
    }
  }

  m_IsPlaying = false;
  m_buttons[ID_MIDI_PLAYER_PLAY]->Display(false);
  m_buttons[ID_MIDI_PLAYER_PAUSE]->Display(false);
  UpdateDisplay();
  m_OrganController->DeleteTimer(this);
}

bool GOMidiPlayer::IsPlaying() { return m_IsPlaying; }

void GOMidiPlayer::UpdateDisplay() {
  if (!IsLoaded())
    m_PlayingTime.SetContent(_("<no file loaded>"));
  else if (!IsPlaying())
    m_PlayingTime.SetContent(_("-:--:--"));
  else
    m_PlayingTime.SetContent(wxString::Format(
      _("%d:%02d:%02d"),
      m_PlayingSeconds / 3600,
      (m_PlayingSeconds / 60) % 60,
      m_PlayingSeconds % 60));
}

void GOMidiPlayer::HandleTimer() {
  if (!m_IsPlaying)
    return;
  GOTime now = wxGetLocalTimeMillis();
  if (m_Start + m_Speed * (m_PlayingSeconds + 1) * 1000 <= now) {
    m_PlayingSeconds++;
    UpdateDisplay();
  }
  do {
    GOMidiEvent e = m_content.GetCurrentEvent();
    if (e.GetTime() * m_Speed + m_Start <= now) {
      if (!m_content.Next()) {
        StopPlaying();
        return;
      }
      e.SetDevice(m_DeviceID);
      e.SetTime(wxGetLocalTimeMillis());
      m_OrganController->ProcessMidi(e);
    } else {
      GOTime next = e.GetTime() * m_Speed + m_Start;
      if (next > m_Start + m_Speed * (m_PlayingSeconds + 1) * 1000)
        next = m_Start + m_Speed * (m_PlayingSeconds + 1) * 1000;
      m_OrganController->SetTimer(next, this);
      return;
    }
  } while (true);
}

GOEnclosure *GOMidiPlayer::GetEnclosure(const wxString &name, bool is_panel) {
  return NULL;
}

GOLabelControl *GOMidiPlayer::GetLabelControl(
  const wxString &name, bool is_panel) {
  if (is_panel)
    return NULL;

  if (name == wxT("MidiPlayerLabel"))
    return &m_PlayingTime;
  return NULL;
}
