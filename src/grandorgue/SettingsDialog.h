/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <vector>
#include <wx/propdlg.h>

#include "SettingsReason.h"

class GOrgueSound;
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
  
  GOrgueSound& m_Sound;
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
  
  SettingsDialog(wxWindow* parent, GOrgueSound& sound, SettingsReasons *reasons);

  bool NeedReload();
  bool NeedRestart();
  
  DECLARE_EVENT_TABLE()
};


#endif
