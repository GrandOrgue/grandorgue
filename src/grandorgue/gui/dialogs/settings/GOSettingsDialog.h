/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSDIALOG_H
#define GOSETTINGSDIALOG_H

#include "gui/dialogs/common/GOTabbedDialog.h"

#include "GOSettingsReason.h"

class GOConfig;
class GOMidiSystem;
class GOSoundSystem;
class GOSettingsAudioGroup;
class GOSettingsAudio;
class GOSettingsMetronome;
class SettingsMidiDevices;
class GOSettingsMidiInitial;
class GOSettingsOptions;
class GOSettingsPaths;
class GOSettingsOrgans;
class GOSettingsReverb;
class GOSettingsTemperaments;

class GOSettingsDialog : public GOTabbedDialog {
public:
  static const wxString PAGE_OPTIONS;
  static const wxString PAGE_PATHS;
  static const wxString PAGE_AUDIO;
  static const wxString PAGE_MIDI_DEVICES;
  static const wxString PAGE_INITIAL_MIDI;
  static const wxString PAGE_ORGANS;
  static const wxString PAGE_REVERB;
  static const wxString PAGE_TEMPERAMENTS;
  static const wxString PAGE_METRONOME;

private:
  enum { ID_REASONS = 100 };

  bool m_ReasonsAlreadyShown;
  SettingsReasons *m_Reasons;
  GOSettingsOptions *m_OptionsPage;
  GOSettingsPaths *m_PathsPage;
  GOSettingsAudio *m_AudioPage;
  SettingsMidiDevices *m_MidiDevicePage;
  GOSettingsMidiInitial *m_MidiMessagePage;
  GOSettingsOrgans *m_OrgansPage;
  GOSettingsReverb *m_ReverbPage;
  GOSettingsTemperaments *m_TemperamentsPage;
  GOSettingsMetronome *m_MetronomePage;

  void OnShow() override;

  void OnReasons(wxCommandEvent &event);

public:
  GOSettingsDialog(
    wxWindow *parent,
    GOConfig &config,
    GOSoundSystem &sound,
    GOMidiSystem &midi,
    SettingsReasons *reasons);

  bool NeedReload();
  bool NeedRestart();

  DECLARE_EVENT_TABLE()
};

#endif
