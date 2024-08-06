/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOAudioDeviceConfig.h"

#include <algorithm>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

const wxString GOAudioDeviceConfig::WX_AUDIO_DEVICES = wxT("AudioDevices");

GOAudioDeviceConfig::GOAudioDeviceConfig(
  const GOAudioDeviceNode &node, uint8_t nChannels)
  : GOAudioDeviceNode(node), m_NChannels(nChannels) {
  ResizeChannels();
}

GOAudioDeviceConfig::GOAudioDeviceConfig(
  const std::vector<wxString> &audioGroups)
  : m_NChannels(2) {
  ResizeChannels();
  std::vector<GroupOutput> &leftOutput = m_ChannelOutputs[0];
  std::vector<GroupOutput> &rightOutput = m_ChannelOutputs[1];
  for (const auto &groupName : audioGroups) {
    leftOutput.emplace_back(groupName, true, DEFAULT_VOLUME);
    rightOutput.emplace_back(groupName, false, DEFAULT_VOLUME);
  }
}

static const wxString WX_DVICE_03D_FMT = wxT("Device%03d");
static const wxString WX_CHANNEL_COUNT = wxT("ChannelCount");
static const wxString WX_CHANNEL_03D_FMT = wxT("Channel%03d");
static const wxString WX_GROUP_COUNT = wxT("GroupCount");
static const wxString WX_GROUP_03D_FMT = wxT("Group%03d");
static const wxString WX_NAME = wxT("Name");
static const wxString WX_LEFT = wxT("Left");
static const wxString WX_RIGHT = wxT("Right");

void GOAudioDeviceConfig::Load(GOConfigReader &cfg, unsigned deviceN) {
  const wxString p0 = wxString::Format(WX_DVICE_03D_FMT, deviceN);

  LoadDeviceNode(cfg, WX_AUDIO_DEVICES, p0);

  m_NChannels = (uint8_t)cfg.ReadInteger(
    CMBSetting, WX_AUDIO_DEVICES, p0 + WX_CHANNEL_COUNT, 0, 200);

  ResizeChannels();
  for (uint8_t j = 0; j < m_NChannels; j++) {
    const wxString p1 = p0 + wxString::Format(WX_CHANNEL_03D_FMT, j + 1);
    std::vector<GroupOutput> &groups = m_ChannelOutputs[j];
    const unsigned groupCount = cfg.ReadInteger(
      CMBSetting, WX_AUDIO_DEVICES, p1 + WX_GROUP_COUNT, 0, 200);

    for (unsigned k = 0; k < groupCount; k++) {
      const wxString p2 = p1 + wxString::Format(WX_GROUP_03D_FMT, k + 1);

      groups.emplace_back(
        cfg.ReadString(CMBSetting, WX_AUDIO_DEVICES, p2 + WX_NAME),
        (float)cfg.ReadFloat(
          CMBSetting, WX_AUDIO_DEVICES, p2 + WX_LEFT, MUTE_VOLUME, MAX_VOLUME),
        (float)cfg.ReadFloat(
          CMBSetting,
          WX_AUDIO_DEVICES,
          p2 + WX_RIGHT,
          MUTE_VOLUME,
          MAX_VOLUME));
    }
  }
}

void GOAudioDeviceConfig::Save(GOConfigWriter &cfg, unsigned deviceN) const {
  const wxString p0 = wxString::Format(WX_DVICE_03D_FMT, deviceN);

  SaveDeviceNode(cfg, WX_AUDIO_DEVICES, p0);
  cfg.WriteInteger(WX_AUDIO_DEVICES, p0 + WX_CHANNEL_COUNT, m_NChannels);
  for (uint8_t j = 0; j < m_NChannels; j++) {
    const wxString p1 = p0 + wxString::Format(WX_CHANNEL_03D_FMT, j + 1);
    const std::vector<GroupOutput> &groups = m_ChannelOutputs[j];
    const unsigned groupCount = groups.size();

    cfg.WriteInteger(WX_AUDIO_DEVICES, p1 + WX_GROUP_COUNT, groupCount);
    for (unsigned k = 0; k < groupCount; k++) {
      const GroupOutput &group = groups[k];
      const wxString p2 = p1 + wxString::Format(WX_GROUP_03D_FMT, k + 1);

      cfg.WriteString(WX_AUDIO_DEVICES, p2 + WX_NAME, group.GetName());
      cfg.WriteFloat(WX_AUDIO_DEVICES, p2 + WX_LEFT, group.GetLeft());
      cfg.WriteFloat(WX_AUDIO_DEVICES, p2 + WX_RIGHT, group.GetRight());
    }
  }
}

void GOAudioDeviceConfig::SetOutputVolume(
  uint8_t channel, const wxString &groupName, bool isLeft, float vol) {
  auto &groups = m_ChannelOutputs[channel];
  auto groupsEnd = groups.end();
  auto iExistingGroup
    = std::find_if(groups.begin(), groups.end(), [&groupName](const auto &g) {
        return g.GetName() == groupName;
      });

  if (iExistingGroup == groupsEnd)
    groups.emplace_back(
      groupName, volumeFor(true, isLeft, vol), volumeFor(false, isLeft, vol));
  else
    iExistingGroup->SetVolume(isLeft, vol);
}
