/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsAudio.h"

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>

#include "config/GOConfig.h"
#include "sound/GOSound.h"
#include "sound/ports/GOSoundPortFactory.h"

class AudioItemData : public wxTreeItemData {
public:
  typedef enum { ROOT_NODE, AUDIO_NODE, CHANNEL_NODE, GROUP_NODE } node_type;

  AudioItemData() {
    type = ROOT_NODE;
    name = wxEmptyString;
    channel = 0;
    latency = 0;
    left = false;
    volume = -121;
  }

  AudioItemData(const wxString &device_name, unsigned desired_latency) {
    type = AUDIO_NODE;
    name = device_name;
    channel = 0;
    latency = desired_latency;
    left = false;
    volume = -121;
  }

  AudioItemData(int ch) {
    type = CHANNEL_NODE;
    name = wxEmptyString;
    channel = ch;
    latency = 0;
    left = false;
    volume = -121;
  }

  AudioItemData(const wxString &group_name, bool left_channel, float vol) {
    type = GROUP_NODE;
    name = group_name;
    channel = 0;
    latency = 0;
    left = left_channel;
    volume = vol;
  }

  node_type type;
  wxString name;
  unsigned channel;
  unsigned latency;
  bool left;
  float volume;
};

/* implementation of wxTreeCtrl that allows resizing beyong the size of all
 * items */
class AudioTreeCtrl : public wxTreeCtrl {
public:
  AudioTreeCtrl(wxWindow *parent, int id, const wxSize &minSize)
    : wxTreeCtrl(
      parent, id, wxDefaultPosition, minSize, wxTR_HAS_BUTTONS | wxTR_SINGLE) {}

  virtual wxSize DoGetBestSize() const override { return GetMinSize(); }
};

BEGIN_EVENT_TABLE(GOSettingsAudio, wxPanel)
EVT_LISTBOX(ID_AUDIOGROUP_LIST, GOSettingsAudio::OnGroup)
EVT_LISTBOX_DCLICK(ID_AUDIOGROUP_LIST, GOSettingsAudio::OnGroupRename)
EVT_BUTTON(ID_AUDIOGROUP_ADD, GOSettingsAudio::OnGroupAdd)
EVT_BUTTON(ID_AUDIOGROUP_DEL, GOSettingsAudio::OnGroupDel)
EVT_BUTTON(ID_AUDIOGROUP_RENAME, GOSettingsAudio::OnGroupRename)
EVT_TREE_SEL_CHANGED(ID_OUTPUT_LIST, GOSettingsAudio::OnOutputChanged)
EVT_BUTTON(ID_OUTPUT_ADD, GOSettingsAudio::OnOutputAdd)
EVT_BUTTON(ID_OUTPUT_DEL, GOSettingsAudio::OnOutputDel)
EVT_BUTTON(ID_OUTPUT_CHANGE, GOSettingsAudio::OnOutputChange)
EVT_BUTTON(ID_OUTPUT_PROPERTIES, GOSettingsAudio::OnOutputProperties)
EVT_BUTTON(ID_OUTPUT_DEFAULT, GOSettingsAudio::OnOutputDefault)
END_EVENT_TABLE()

GOSettingsAudio::GOSettingsAudio(
  GOConfig &config, GOSound &sound, wxWindow *parent)
  : wxPanel(parent, wxID_ANY),
    GOSettingsPorts(this, GOSoundPortFactory::getInstance(), _("Sound &ports")),
    m_config(config),
    m_Sound(sound) {
  wxGridBagSizer *const gridRoot = new wxGridBagSizer(0, 0);

  wxBoxSizer *const boxAudioGroups
    = new wxStaticBoxSizer(wxVERTICAL, this, _("Audio groups"));
  wxGridBagSizer *gridGroups = new wxGridBagSizer(0, 0);

  m_AudioGroups = new wxListBox(
    this,
    ID_AUDIOGROUP_LIST,
    wxDefaultPosition,
    wxDefaultSize,
    0,
    NULL,
    wxLB_SINGLE | wxLB_NEEDED_SB);
  gridGroups->Add(m_AudioGroups, wxGBPosition(0, 0), wxGBSpan(1, 4), wxEXPAND);
  m_AddGroup = new wxButton(this, ID_AUDIOGROUP_ADD, _("&Add"));
  gridGroups->Add(m_AddGroup, wxGBPosition(1, 0), wxDefaultSpan, wxALL, 5);
  m_DelGroup = new wxButton(this, ID_AUDIOGROUP_DEL, _("&Delete"));
  m_DelGroup->Disable();
  gridGroups->Add(m_DelGroup, wxGBPosition(1, 1), wxDefaultSpan, wxALL, 5);
  m_RenameGroup = new wxButton(this, ID_AUDIOGROUP_RENAME, _("Rename"));
  m_RenameGroup->Disable();
  gridGroups->Add(m_RenameGroup, wxGBPosition(1, 2), wxDefaultSpan, wxALL, 5);

  gridGroups->AddGrowableRow(0, 1);
  gridGroups->AddGrowableCol(3, 1);
  boxAudioGroups->Add(gridGroups, 1, wxEXPAND);

  gridRoot->Add(
    boxAudioGroups, wxGBPosition(0, 0), wxGBSpan(2, 1), wxEXPAND | wxALL, 5);

  wxBoxSizer *const boxSound
    = new wxStaticBoxSizer(wxHORIZONTAL, this, _("&Sound output"));
  wxFlexGridSizer *gridOutput = new wxFlexGridSizer(2, 5, 5);

  gridOutput->Add(
    new wxStaticText(this, wxID_ANY, _("Sample Rate:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  gridOutput->Add(m_SampleRate = new wxChoice(this, ID_SAMPLE_RATE));
  gridOutput->Add(
    new wxStaticText(this, wxID_ANY, _("Samples per buffer:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  gridOutput->Add(
    m_SamplesPerBuffer = new wxSpinCtrl(
      this,
      ID_SAMPLES_PER_BUFFER,
      wxEmptyString,
      wxDefaultPosition,
      wxSize(130, wxDefaultCoord)));

  boxSound->Add(gridOutput, 1, wxEXPAND | wxALL, 5);
  gridRoot->Add(
    boxSound, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND | wxALL, 5);

  gridRoot->Add(
    GetPortsBox(), wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND | wxALL, 5);

  wxBoxSizer *const boxMap
    = new wxStaticBoxSizer(wxVERTICAL, this, _("&Mapping output"));
  wxGridBagSizer *gridMap = new wxGridBagSizer(0, 0);

  m_AudioOutput = new AudioTreeCtrl(this, ID_OUTPUT_LIST, wxDefaultSize);
  gridMap->Add(
    m_AudioOutput,
    wxGBPosition(0, 0),
    wxGBSpan(1, 6),
    wxEXPAND | wxLEFT | wxRIGHT,
    5);
  m_AddMap = new wxButton(this, ID_OUTPUT_ADD, _("&Add"));
  gridMap->Add(m_AddMap, wxGBPosition(1, 0), wxDefaultSpan, wxALL, 5);
  m_DelMap = new wxButton(this, ID_OUTPUT_DEL, _("&Delete"));
  gridMap->Add(m_DelMap, wxGBPosition(1, 1), wxDefaultSpan, wxALL, 5);
  m_ChangeMap = new wxButton(this, ID_OUTPUT_CHANGE, _("Change"));
  gridMap->Add(m_ChangeMap, wxGBPosition(1, 2), wxDefaultSpan, wxALL, 5);
  m_PropertiesMap = new wxButton(this, ID_OUTPUT_PROPERTIES, _("Properties"));
  gridMap->Add(m_PropertiesMap, wxGBPosition(1, 3), wxDefaultSpan, wxALL, 5);
  m_DefaultMap = new wxButton(this, ID_OUTPUT_DEFAULT, _("Revert to Default"));
  gridMap->Add(m_DefaultMap, wxGBPosition(1, 4), wxDefaultSpan, wxALL, 5);
  gridMap->AddGrowableRow(0, 1);
  gridMap->AddGrowableCol(5, 1);
  boxMap->Add(gridMap, 1, wxEXPAND);

  gridRoot->Add(
    boxMap, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 5);
  gridRoot->AddGrowableRow(1, 1);
  gridRoot->AddGrowableRow(2, 3);
  gridRoot->AddGrowableCol(0, 1);

  SetSizerAndFit(gridRoot);
}

bool GOSettingsAudio::TransferDataToWindow() {
  std::vector<wxString> audio_groups = m_config.GetAudioGroups();
  for (unsigned i = 0; i < audio_groups.size(); i++)
    m_AudioGroups->Append(audio_groups[i]);

  m_SampleRate->Append(wxT("44100"));
  m_SampleRate->Append(wxT("48000"));
  m_SampleRate->Append(wxT("96000"));
  m_SampleRate->Select(0);
  for (unsigned i = 0; i < m_SampleRate->GetCount(); i++)
    if (
      wxString::Format(wxT("%d"), m_config.SampleRate())
      == m_SampleRate->GetString(i))
      m_SampleRate->Select(i);
  m_SamplesPerBuffer->SetRange(1, MAX_FRAME_SIZE);
  m_SamplesPerBuffer->SetValue(m_config.SamplesPerBuffer());

  FillPortsWith(m_config.GetSoundPortsConfig());
  m_AudioOutput->AddRoot(_("Audio Output"), -1, -1, new AudioItemData());
  std::vector<GOAudioDeviceConfig> audio_config
    = m_Sound.GetSettings().GetAudioDeviceConfig();
  for (unsigned i = 0; i < audio_config.size(); i++) {
    wxTreeItemId audio
      = AddDeviceNode(audio_config[i].name, audio_config[i].desired_latency);
    for (unsigned j = 0; j < audio_config[i].channels; j++) {
      wxTreeItemId channel = AddChannelNode(audio, j);
      if (j >= audio_config[i].scale_factors.size())
        continue;
      for (unsigned k = 0; k < audio_config[i].scale_factors[j].size(); k++) {
        if (audio_config[i].scale_factors[j][k].left >= -120) {
          wxTreeItemId group;
          group = AddGroupNode(
            channel, audio_config[i].scale_factors[j][k].name, true);
          UpdateVolume(group, audio_config[i].scale_factors[j][k].left);
        }
        if (audio_config[i].scale_factors[j][k].right >= -120) {
          wxTreeItemId group;
          group = AddGroupNode(
            channel, audio_config[i].scale_factors[j][k].name, false);
          UpdateVolume(group, audio_config[i].scale_factors[j][k].right);
        }
      }
    }
  }
  m_AudioOutput->ExpandAll();
  UpdateButtons();
  return true;
}

AudioItemData *GOSettingsAudio::GetObject(const wxTreeItemId &id) {
  if (!id.IsOk())
    return NULL;
  return (AudioItemData *)m_AudioOutput->GetItemData(id);
}

wxTreeItemId GOSettingsAudio::GetDeviceNode(const wxString &name) {
  wxTreeItemIdValue i;
  wxTreeItemId current;
  wxTreeItemId root = m_AudioOutput->GetRootItem();
  current = m_AudioOutput->GetFirstChild(root, i);
  while (current.IsOk()) {
    AudioItemData *data = (AudioItemData *)m_AudioOutput->GetItemData(current);
    if (data && data->type == AudioItemData::AUDIO_NODE && data->name == name)
      return current;
    current = m_AudioOutput->GetNextChild(root, i);
  }
  return current;
}

wxTreeItemId GOSettingsAudio::GetChannelNode(
  const wxTreeItemId &audio, unsigned channel) {
  wxTreeItemIdValue i;
  wxTreeItemId current;
  current = m_AudioOutput->GetFirstChild(audio, i);
  while (current.IsOk()) {
    AudioItemData *data = (AudioItemData *)m_AudioOutput->GetItemData(current);
    if (
      data && data->type == AudioItemData::CHANNEL_NODE
      && data->channel == channel)
      return current;
    current = m_AudioOutput->GetNextChild(audio, i);
  }
  return current;
}

wxTreeItemId GOSettingsAudio::GetGroupNode(
  const wxTreeItemId &channel, const wxString &name, bool left) {
  wxTreeItemIdValue i;
  wxTreeItemId current;
  current = m_AudioOutput->GetFirstChild(channel, i);
  while (current.IsOk()) {
    AudioItemData *data = (AudioItemData *)m_AudioOutput->GetItemData(current);
    if (
      data && data->type == AudioItemData::GROUP_NODE && data->name == name
      && data->left == left)
      return current;
    current = m_AudioOutput->GetNextChild(channel, i);
  }
  return current;
}

wxTreeItemId GOSettingsAudio::AddDeviceNode(wxString name) {
  return AddDeviceNode(name, m_Sound.GetSettings().GetDefaultLatency());
}

wxTreeItemId GOSettingsAudio::AddDeviceNode(
  wxString name, unsigned desired_latency) {
  wxTreeItemId current;
  if (name == wxEmptyString) {
    name = m_Sound.GetDefaultAudioDevice(RenewPortsConfig());
  }
  current = GetDeviceNode(name);
  if (current.IsOk())
    return current;
  current = m_AudioOutput->AppendItem(
    m_AudioOutput->GetRootItem(),
    wxEmptyString,
    -1,
    -1,
    new AudioItemData(name, desired_latency));
  UpdateDevice(current);
  m_AudioOutput->Expand(current);
  return current;
}

wxTreeItemId GOSettingsAudio::AddChannelNode(
  const wxTreeItemId &audio, unsigned channel) {
  wxTreeItemId current;
  current = GetChannelNode(audio, channel);
  if (current.IsOk())
    return current;
  current = m_AudioOutput->AppendItem(
    audio,
    wxString::Format(_("Channel %d"), channel + 1),
    -1,
    -1,
    new AudioItemData(channel));
  m_AudioOutput->Expand(current);
  m_AudioOutput->Expand(audio);
  return current;
}

wxTreeItemId GOSettingsAudio::AddGroupNode(
  const wxTreeItemId &channel, const wxString &name, bool left) {
  wxTreeItemId current;
  current = GetGroupNode(channel, name, left);
  if (current.IsOk())
    return current;
  current = m_AudioOutput->AppendItem(
    channel, name, -1, -1, new AudioItemData(name, left, -121));
  m_AudioOutput->Expand(current);
  m_AudioOutput->Expand(channel);
  UpdateVolume(current, -121);
  return current;
}

void GOSettingsAudio::UpdateDevice(const wxTreeItemId &dev) {
  AudioItemData *data = GetObject(dev);
  wxString text = wxString::Format(
    _("Device: %s [%d ms requested]"), data->name.c_str(), data->latency);
  m_AudioOutput->SetItemText(dev, text);
}

void GOSettingsAudio::UpdateVolume(const wxTreeItemId &group, float volume) {
  AudioItemData *data = GetObject(group);
  wxString name = wxString::Format(
    data->left ? _("%s - left") : _("%s - right"), data->name.c_str());
  if (volume < -121.0 || volume > 40.0)
    volume = 0;
  data->volume = volume;
  if (volume >= -120)
    m_AudioOutput->SetItemText(
      group, wxString::Format(_("%s: %f dB"), name.c_str(), volume));
  else
    m_AudioOutput->SetItemText(
      group, wxString::Format(_("%s: mute"), name.c_str()));
}

void GOSettingsAudio::AssureDeviceList() {
  const GOPortsConfig portsConfig(RenewPortsConfig());

  if (m_PortsConfigPopulatedWith != portsConfig) {
    m_DeviceList = m_Sound.GetAudioDevices(portsConfig);
    m_PortsConfigPopulatedWith = portsConfig;
  }
}

std::vector<wxString> GOSettingsAudio::GetRemainingAudioDevices(
  const wxTreeItemId *ignoreItem) {
  AssureDeviceList();

  std::vector<wxString> result;

  for (unsigned i = 0; i < m_DeviceList.size(); i++) {
    const wxTreeItemId item = GetDeviceNode(m_DeviceList[i].name);

    if (
      !item.IsOk()
      || (ignoreItem != NULL && item.GetID() == ignoreItem->GetID()))
      result.push_back(m_DeviceList[i].name);
  }
  return result;
}

std::vector<std::pair<wxString, bool>> GOSettingsAudio::GetRemainingAudioGroups(
  const wxTreeItemId &channel) {
  std::vector<std::pair<wxString, bool>> result;

  for (unsigned l = m_AudioGroups->GetCount(), i = 0; i < l; i++) {
    const wxString &name = m_AudioGroups->GetString(i);

    if (!GetGroupNode(channel, name, true).IsOk())
      result.push_back(std::pair<wxString, bool>(name, true));
    if (!GetGroupNode(channel, name, false).IsOk())
      result.push_back(std::pair<wxString, bool>(name, false));
  }
  return result;
}
void GOSettingsAudio::OnGroup(wxCommandEvent &event) {
  if (m_AudioGroups->GetSelection() != wxNOT_FOUND)
    m_RenameGroup->Enable();
  else
    m_RenameGroup->Disable();

  if (m_AudioGroups->GetCount() > 1)
    m_DelGroup->Enable();
  else
    m_DelGroup->Disable();
}

void GOSettingsAudio::OnGroupAdd(wxCommandEvent &event) {
  wxString str
    = wxGetTextFromUser(_("New audio group name"), _("Audio groups")).Trim();
  if (str != wxEmptyString)
    if (m_AudioGroups->FindString(str) == wxNOT_FOUND)
      m_AudioGroups->Append(str);
}

void GOSettingsAudio::OnGroupDel(wxCommandEvent &event) {
  if (m_AudioGroups->GetCount() > 1)
    m_AudioGroups->Delete(m_AudioGroups->GetSelection());
  m_AudioGroups->SetSelection(0);
}

void GOSettingsAudio::OnGroupRename(wxCommandEvent &event) {
  int index = m_AudioGroups->GetSelection();
  wxString str = wxGetTextFromUser(
                   _("Audio group name"),
                   _("Audio groups"),
                   m_AudioGroups->GetString(index))
                   .Trim();
  if (str != wxEmptyString) {
    int idx = m_AudioGroups->FindString(str);
    if (idx == wxNOT_FOUND)
      m_AudioGroups->SetString(index, str);
    else if (idx != index)
      m_AudioGroups->Delete(index);
  }
}

void GOSettingsAudio::UpdateButtons() {
  wxTreeItemId selection = m_AudioOutput->GetSelection();
  AudioItemData *data = GetObject(selection);
  if (data && data->type == AudioItemData::AUDIO_NODE) {
    /*
    bool enable = false;
    for(unsigned i = 0; i < m_DeviceList.size(); i++)
            if (m_DeviceList[i].name == data->name)
                    if (m_AudioOutput->GetChildrenCount(selection, false) <
    m_DeviceList[i].channels) enable = true; if (enable) m_Add->Enable(); else
            m_Add->Disable();

     */
    m_AddMap->Enable();
    m_PropertiesMap->Enable();
    m_ChangeMap->Enable();
    if (
      m_AudioOutput->GetChildrenCount(m_AudioOutput->GetRootItem(), false) > 1)
      m_DelMap->Enable();
    else
      m_DelMap->Disable();
  } else if (data && data->type == AudioItemData::CHANNEL_NODE) {
    m_PropertiesMap->Disable();
    m_ChangeMap->Disable();
    if (GetRemainingAudioGroups(selection).size())
      m_AddMap->Enable();
    else
      m_AddMap->Disable();
    if (
      m_AudioOutput->GetLastChild(m_AudioOutput->GetItemParent(selection))
      == selection) {
      if (data->channel > 0)
        m_DelMap->Enable();
      else
        m_DelMap->Disable();
    } else
      m_DelMap->Disable();
  } else if (data && data->type == AudioItemData::GROUP_NODE) {
    m_PropertiesMap->Enable();
    m_ChangeMap->Enable();
    m_AddMap->Disable();
    m_DelMap->Enable();
  } else if (data && data->type == AudioItemData::ROOT_NODE) {
    m_PropertiesMap->Disable();
    m_ChangeMap->Disable();
    m_AddMap->Enable();
    m_DelMap->Disable();
  } else {
    m_PropertiesMap->Disable();
    m_ChangeMap->Disable();
    m_AddMap->Disable();
    m_DelMap->Disable();
  }
}

void GOSettingsAudio::OnOutputChanged(wxTreeEvent &event) { UpdateButtons(); }

void GOSettingsAudio::OnOutputAdd(wxCommandEvent &event) {
  wxTreeItemId selection = m_AudioOutput->GetSelection();
  AudioItemData *data = GetObject(selection);
  if (data && data->type == AudioItemData::AUDIO_NODE) {
    unsigned channels = m_AudioOutput->GetChildrenCount(selection, false);
    /*
    for(unsigned i = 0; i < m_DeviceList.size(); i++)
            if (m_DeviceList[i].name == data->name)
                    if (channels < m_DeviceList[i].channels) */
    AddChannelNode(selection, channels);
  } else if (data && data->type == AudioItemData::CHANNEL_NODE) {
    std::vector<std::pair<wxString, bool>> groups;
    wxArrayString names;
    int index;
    groups = GetRemainingAudioGroups(selection);
    for (unsigned i = 0; i < groups.size(); i++)
      names.Add(wxString::Format(
        groups[i].second ? _("%s - left") : _("%s - right"),
        groups[i].first.c_str()));
    index = wxGetSingleChoiceIndex(
      _("Add new audio group"), _("New audio group"), names, this);
    if (index == -1)
      return;
    wxTreeItemId id
      = AddGroupNode(selection, groups[index].first, groups[index].second);
    UpdateVolume(id, 0);
  } else if (data && data->type == AudioItemData::ROOT_NODE) {
    int index;
    wxArrayString devs;
    std::vector<wxString> devices = GetRemainingAudioDevices(NULL);
    for (unsigned i = 0; i < devices.size(); i++)
      devs.Add(devices[i]);
    index = wxGetSingleChoiceIndex(
      _("Add new audio device"), _("New audio device"), devs, this);
    if (index == -1)
      return;
    wxMessageBox(
      _("Using more than one audio interface is currently not supported."),
      _("Warning"),
      wxOK | wxICON_WARNING,
      this);
    wxTreeItemId audio = AddDeviceNode(devices[index]);
    AddChannelNode(audio, 0);
  }
  UpdateButtons();
}

void GOSettingsAudio::OnOutputDel(wxCommandEvent &event) {
  wxTreeItemId selection = m_AudioOutput->GetSelection();
  AudioItemData *data = GetObject(selection);
  if (data && data->type == AudioItemData::AUDIO_NODE) {
    if (
      m_AudioOutput->GetChildrenCount(m_AudioOutput->GetRootItem(), false) > 1)
      m_AudioOutput->Delete(selection);
  } else if (data && data->type == AudioItemData::CHANNEL_NODE) {
    m_AudioOutput->Delete(selection);
  } else if (data && data->type == AudioItemData::GROUP_NODE) {
    m_AudioOutput->Delete(selection);
  }
  UpdateButtons();
}

void GOSettingsAudio::OnOutputChange(wxCommandEvent &event) {
  wxTreeItemId selection = m_AudioOutput->GetSelection();
  AudioItemData *data = GetObject(selection);
  if (data && data->type == AudioItemData::AUDIO_NODE) {
    int index;
    wxArrayString devs;
    std::vector<wxString> devices = GetRemainingAudioDevices(&selection);

    int initialSelection = 0;

    for (unsigned i = 0; i < devices.size(); i++) {
      const wxString devName = devices[i];

      devs.Add(devName);
      if (devName == data->name)
        initialSelection = i;
    }
    index = wxGetSingleChoiceIndex(
      _("Change audio device"),
      _("Change audio device"),
      devs,
      initialSelection,
      this);
    if (index <= -1 || devs[index] == data->name)
      return;
    unsigned channels = m_AudioOutput->GetChildrenCount(selection, false);
    bool error = false;
    for (unsigned i = 0; i < m_DeviceList.size(); i++)
      if (m_DeviceList[i].name == devs[index])
        if (channels > m_DeviceList[i].channels)
          error = true;
    if (error) {
      wxMessageBox(
        _("Too many audio channels configured for the new audio interface"),
        _("Error"),
        wxOK | wxICON_ERROR,
        this);
      return;
    }
    data->name = devs[index];

    UpdateDevice(selection);
  } else if (data && data->type == AudioItemData::GROUP_NODE) {
    int index;
    std::vector<std::pair<wxString, bool>> groups;
    wxArrayString names;
    groups = GetRemainingAudioGroups(m_AudioOutput->GetItemParent(selection));
    groups.insert(
      groups.begin(), std::pair<wxString, bool>(data->name, data->left));
    for (unsigned i = 0; i < groups.size(); i++)
      names.Add(wxString::Format(
        groups[i].second ? _("%s - left") : _("%s - right"),
        groups[i].first.c_str()));
    index = wxGetSingleChoiceIndex(
      _("Change audio group"), _("Change audio group"), names, this);
    if (index == -1 || index == 0)
      return;
    data->name = groups[index].first;
    data->left = groups[index].second;
    UpdateVolume(selection, data->volume);
  }
  UpdateButtons();
}

void GOSettingsAudio::OnOutputProperties(wxCommandEvent &event) {
  wxTreeItemId selection = m_AudioOutput->GetSelection();
  AudioItemData *data = GetObject(selection);
  if (data && data->type == AudioItemData::AUDIO_NODE) {
    int latency = data->latency;
    latency = wxGetNumberFromUser(
      _("Desired output latency"),
      _("Desired latency:"),
      _("Audio device settings"),
      latency,
      0,
      999,
      this);
    if (latency == -1)
      return;
    data->latency = latency;
    UpdateDevice(selection);
  } else if (data && data->type == AudioItemData::GROUP_NODE) {
    wxString current
      = wxString::Format(wxT("%f"), data->volume >= -120 ? data->volume : -120);
    current = wxGetTextFromUser(
      _("Please enter new volume in dB:"), _("Change audio group"), current);
    if (current == wxEmptyString)
      return;
    double volume;
    if (!current.ToDouble(&volume) || volume < -120.0 || volume > 40.0) {
      wxMessageBox(
        _("Please enter a volume between -120 and 40 dB"),
        _("Error"),
        wxOK | wxICON_ERROR,
        this);
      return;
    }
    data->volume = volume;
    UpdateVolume(selection, data->volume);
  }
  UpdateButtons();
}

void GOSettingsAudio::OnOutputDefault(wxCommandEvent &event) {
  if (
    wxMessageBox(
      _("Should the audio config be reverted to the default "
        "stereo configuration?"),
      _("Revert"),
      wxYES_NO,
      this)
    == wxNO)
    return;
  wxTreeItemId root = m_AudioOutput->GetRootItem();
  wxTreeItemId audio, channel, group;
  wxTreeItemIdValue i;
  wxString dev_name = wxEmptyString;
  audio = m_AudioOutput->GetFirstChild(root, i);
  while (audio.IsOk()) {
    if (dev_name == wxEmptyString)
      dev_name = ((AudioItemData *)m_AudioOutput->GetItemData(audio))->name;
    audio = m_AudioOutput->GetNextChild(root, i);
  }
  m_AudioOutput->DeleteChildren(root);

  audio = AddDeviceNode(dev_name);
  channel = AddChannelNode(audio, 0);

  for (unsigned l = m_AudioGroups->GetCount(), i = 0; i < l; i++) {
    group = AddGroupNode(channel, m_AudioGroups->GetString(i), true);
    UpdateVolume(group, 0);
  }

  channel = AddChannelNode(audio, 1);

  for (unsigned l = m_AudioGroups->GetCount(), i = 0; i < l; i++) {
    group = AddGroupNode(channel, m_AudioGroups->GetString(i), false);
    UpdateVolume(group, 0);
  }

  m_AudioOutput->ExpandAll();
  UpdateButtons();
}

bool GOSettingsAudio::TransferDataFromWindow() {
  std::vector<wxString> audioGroups;

  for (unsigned l = m_AudioGroups->GetCount(), i = 0; i < l; i++)
    audioGroups.push_back(m_AudioGroups->GetString(i));
  m_config.SetAudioGroups(audioGroups);

  unsigned long sample_rate;

  if (m_SampleRate->GetStringSelection().ToULong(&sample_rate))
    m_config.SampleRate(sample_rate);
  else
    wxLogError(_("Invalid sample rate"));
  m_config.SamplesPerBuffer(m_SamplesPerBuffer->GetValue());

  m_Sound.GetSettings().SetSoundPortsConfig(RenewPortsConfig());

  std::vector<GOAudioDeviceConfig> audio_config;
  wxTreeItemId root = m_AudioOutput->GetRootItem();
  wxTreeItemId audio, channel, group;
  wxTreeItemIdValue i, j, k;

  audio = m_AudioOutput->GetFirstChild(root, i);
  while (audio.IsOk()) {
    GOAudioDeviceConfig conf;
    conf.name = ((AudioItemData *)m_AudioOutput->GetItemData(audio))->name;
    conf.desired_latency
      = ((AudioItemData *)m_AudioOutput->GetItemData(audio))->latency;
    conf.channels = m_AudioOutput->GetChildrenCount(audio, false);
    conf.scale_factors.resize(conf.channels);

    int channel_id = 0;
    channel = m_AudioOutput->GetFirstChild(audio, j);
    while (channel.IsOk()) {
      group = m_AudioOutput->GetFirstChild(channel, k);
      while (group.IsOk()) {
        AudioItemData *data
          = (AudioItemData *)m_AudioOutput->GetItemData(group);
        bool found = false;

        for (unsigned l = 0; l < conf.scale_factors[channel_id].size(); l++)
          if (conf.scale_factors[channel_id][l].name == data->name) {
            found = true;
            if (data->left)
              conf.scale_factors[channel_id][l].left = data->volume;
            else
              conf.scale_factors[channel_id][l].right = data->volume;
          }
        if (!found) {
          GOAudioGroupOutputConfig gconf;
          gconf.name = data->name;
          gconf.left = -121;
          gconf.right = -121;

          if (data->left)
            gconf.left = data->volume;
          else
            gconf.right = data->volume;

          conf.scale_factors[channel_id].push_back(gconf);
        }

        group = m_AudioOutput->GetNextChild(channel, k);
      }

      channel = m_AudioOutput->GetNextChild(audio, j);
      channel_id++;
    }

    audio_config.push_back(conf);
    audio = m_AudioOutput->GetNextChild(root, i);
  }
  m_Sound.GetSettings().SetAudioDeviceConfig(audio_config);
  return true;
}
