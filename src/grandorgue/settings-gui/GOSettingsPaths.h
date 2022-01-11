/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSPATHS_H
#define GOSETTINGSPATHS_H

#include <wx/panel.h>

class GOConfig;
class wxDirPickerCtrl;
class wxSpinCtrl;

class GOSettingsPaths : public wxPanel {
  enum {
    ID_ORGAN_SETTINGS_PATH = 200,
    ID_ORGAN_CACHE_PATH,
    ID_ORGAN_PATH,
    ID_ORGANPACKAGE_PATH,
    ID_SETTING_PATH,
    ID_AUDIO_RECORDER_PATH,
    ID_MIDI_RECORDER_PATH,
    ID_MIDI_PLAYER_PATH,
  };

private:
  GOConfig &m_config;
  wxDirPickerCtrl *m_OrganSettingsPath;
  wxDirPickerCtrl *m_CachePath;
  wxDirPickerCtrl *m_OrganPath;
  wxDirPickerCtrl *m_OrganPackagePath;
  wxDirPickerCtrl *m_SettingPath;
  wxDirPickerCtrl *m_AudioRecorderPath;
  wxDirPickerCtrl *m_MidiRecorderPath;
  wxDirPickerCtrl *m_MidiPlayerPath;

public:
  GOSettingsPaths(GOConfig &settings, wxWindow *parent);

  void Save();
};

#endif
