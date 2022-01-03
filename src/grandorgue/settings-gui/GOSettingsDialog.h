/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <vector>
#include <wx/propdlg.h>

#include "GOSettingsReason.h"

class GOSound;
class SettingsArchives;
class SettingsAudioGroup;
class SettingsAudioOutput;
class SettingsMidiDevices;
class SettingsMidiMessage;
class SettingsOption;
class SettingsDefaults;
class SettingsOrgan;
class SettingsReverb;
class SettingsTemperaments;

class SettingsDialog : public wxPropertySheetDialog
{
private:
  enum {
    ID_REASONS = 100
  };
  
  GOSound& m_Sound;
  bool m_ReasonsAlreadyShown;
  SettingsReasons *m_Reasons;
  SettingsOption* m_OptionsPage;
  SettingsDefaults* m_DefaultsPage;
  SettingsAudioOutput* m_OutputPage;
  SettingsReverb* m_ReverbPage;
  SettingsAudioGroup* m_GroupPage;
  SettingsOrgan* m_OrganPage;
  SettingsMidiDevices* m_MidiDevicePage;
  SettingsTemperaments* m_TemperamentsPage;
  SettingsMidiMessage* m_MidiMessagePage;
  SettingsArchives* m_ArchivePage;

  void OnShow(wxShowEvent &);

  bool DoApply();

  void OnApply(wxCommandEvent& event);
  void OnOK(wxCommandEvent& event);
  void OnHelp(wxCommandEvent& event);
  void OnReasons(wxCommandEvent& event);

public:
  // the order must be the same as the order of pages
  typedef enum {
    PAGE_OPTIONS = 0,
    PAGE_DEFAULTS,
    PAGE_AUDIO_OUTPUT,
    PAGE_REVERB,
    PAGE_AUDIO_GROUPS,
    PAGE_ORGANS,
    PAGE_MIDI_DEVICES,
    PAGE_TEMPERAMENTS,
    PAGE_INIT_MIDI_CONFIG,
    PAGE_ORGAN_PACKAGES
  } PageSelector;
  
  SettingsDialog(wxWindow* parent, GOSound& sound, SettingsReasons *reasons);

  bool NeedReload();
  bool NeedRestart();
  
  DECLARE_EVENT_TABLE()
};


#endif
