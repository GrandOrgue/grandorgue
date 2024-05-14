/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSAUDIO_H
#define GOSETTINGSAUDIO_H

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/string.h>
#include <wx/treectrl.h>

#include <vector>

#include "config/GOConfig.h"
#include "sound/GOSoundDevInfo.h"
#include "sound/ports/GOSoundPort.h"

#include "GOSettingsPorts.h"

class wxButton;
class wxListBox;
class AudioItemData;
class GOSound;

class GOSettingsAudio : public wxPanel, GOSettingsPorts {
  enum {
    ID_AUDIOGROUP_LIST = 200,
    ID_AUDIOGROUP_ADD,
    ID_AUDIOGROUP_DEL,
    ID_AUDIOGROUP_RENAME,
    ID_SAMPLE_RATE,
    ID_SAMPLES_PER_BUFFER,
    ID_SOND_PORTS,
    ID_OUTPUT_LIST,
    ID_OUTPUT_ADD,
    ID_OUTPUT_DEL,
    ID_OUTPUT_CHANGE,
    ID_OUTPUT_PROPERTIES,
    ID_OUTPUT_DEFAULT,
  };

private:
  GOConfig &m_config;
  GOSound &m_Sound;

  wxListBox *m_AudioGroups;
  wxButton *m_AddGroup;
  wxButton *m_DelGroup;
  wxButton *m_RenameGroup;

  wxChoice *m_SampleRate;
  wxSpinCtrl *m_SamplesPerBuffer;

  wxTreeCtrl *m_AudioOutput;
  wxButton *m_AddMap;
  wxButton *m_DelMap;
  wxButton *m_ChangeMap;
  wxButton *m_PropertiesMap;
  wxButton *m_DefaultMap;

  GOPortsConfig m_PortsConfigPopulatedWith;
  std::vector<GOSoundDevInfo> m_DeviceList;

  AudioItemData *GetObject(const wxTreeItemId &id);
  wxTreeItemId GetDeviceNode(const wxString &name);
  wxTreeItemId GetChannelNode(const wxTreeItemId &audio, unsigned channel);
  wxTreeItemId GetGroupNode(
    const wxTreeItemId &channel, const wxString &name, bool left);

  wxTreeItemId AddDeviceNode(wxString name);
  wxTreeItemId AddDeviceNode(wxString name, unsigned latency);
  wxTreeItemId AddChannelNode(const wxTreeItemId &audio, unsigned channel);
  wxTreeItemId AddGroupNode(
    const wxTreeItemId &channel, const wxString &name, bool left);
  void UpdateDevice(const wxTreeItemId &dev);
  void UpdateVolume(const wxTreeItemId &group, float volume);
  void UpdateButtons();

  void AssureDeviceList();
  std::vector<wxString> GetRemainingAudioDevices(
    const wxTreeItemId *ignoreItem);
  std::vector<std::pair<wxString, bool>> GetRemainingAudioGroups(
    const wxTreeItemId &channel);

  void OnGroup(wxCommandEvent &event);
  void OnGroupAdd(wxCommandEvent &event);
  void OnGroupDel(wxCommandEvent &event);
  void OnGroupRename(wxCommandEvent &event);

  void OnOutputChanged(wxTreeEvent &event);
  void OnOutputAdd(wxCommandEvent &event);
  void OnOutputDel(wxCommandEvent &event);
  void OnOutputChange(wxCommandEvent &event);
  void OnOutputProperties(wxCommandEvent &event);
  void OnOutputDefault(wxCommandEvent &event);

public:
  GOSettingsAudio(GOConfig &config, GOSound &sound, wxWindow *parent);

  virtual bool TransferDataToWindow() override;
  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
