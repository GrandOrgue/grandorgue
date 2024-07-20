/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIODEVICECONFIG_H
#define GOAUDIODEVICECONFIG_H

#include <cstdint>
#include <vector>

#include <wx/string.h>

#include "GOAudioDeviceNode.h"

class GOConfigReader;
class GOConfigWriter;

class GOAudioDeviceConfig : public GOAudioDeviceNode {
public:
  // volume values in dB
  static constexpr float MUTE_VOLUME = -121.0f;
  static constexpr float MIN_VOLUME = -120.0f;
  static constexpr float DEFAULT_VOLUME = 0.0f;
  static constexpr float MAX_VOLUME = 40.0f;

  static const wxString WX_AUDIO_DEVICES;

  static bool isEnabled(float vol) { return vol >= MIN_VOLUME; }

  class GroupOutput {
  private:
    wxString m_name;
    float m_left;
    float m_right;

  public:
    GroupOutput(const wxString &name, float left, float right)
      : m_name(name), m_left(left), m_right(right) {}

    const wxString &GetName() const { return m_name; }
    const bool IsLeftEnabled() const { return isEnabled(m_left); }
    const bool IsRightEnabled() const { return isEnabled(m_right); }
    const float GetLeft() const { return m_left; }
    const float GetRight() const { return m_right; }

    void SetVolume(bool isLeft, float vol) {
      (isLeft ? m_left : m_right) = vol;
    }
  };

private:
  uint8_t m_NChannels;
  std::vector<std::vector<GroupOutput>> m_ChannelOutputs;

  static float volumeFor(bool isForLeft, bool isLeft, float vol) {
    return isLeft == isForLeft ? vol : MUTE_VOLUME;
  }

  void ResizeChannels() { m_ChannelOutputs.resize(m_NChannels); }

public:
  GOAudioDeviceConfig(const GOAudioDeviceNode &node, uint8_t nChannels);

  // Creates en ampty config for future Load()
  GOAudioDeviceConfig() {}

  // Creates the config with default settings
  GOAudioDeviceConfig(const std::vector<wxString> &audioGroups);

  uint8_t GetChannels() const { return m_NChannels; }

  const std::vector<std::vector<GroupOutput>> &GetChannelOututs() const {
    return m_ChannelOutputs;
  }

  void Load(GOConfigReader &cfg, unsigned deviceN);
  void Save(GOConfigWriter &cfg, unsigned deviceN) const;

  // sets the volume for the group. If the group is not on file then create it
  void SetOutputVolume(
    uint8_t channel, const wxString &groupName, bool isLeft, float vol);
};

#endif /* GOAUDIODEVICECONFIG_H */
