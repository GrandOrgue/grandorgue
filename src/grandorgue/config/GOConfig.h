/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIG_H
#define GOCONFIG_H

#include <wx/gdicmn.h>
#include <wx/string.h>

#include <map>
#include <vector>

#include "dialogs/common/GODialogSizeSet.h"
#include "midi/GOMidiMap.h"
#include "midi/GOMidiReceiverBase.h"
#include "settings/GOSettingBool.h"
#include "settings/GOSettingDirectory.h"
#include "settings/GOSettingEnum.h"
#include "settings/GOSettingFile.h"
#include "settings/GOSettingFloat.h"
#include "settings/GOSettingNumber.h"
#include "settings/GOSettingStore.h"
#include "settings/GOSettingString.h"
#include "size/GOLogicalRect.h"
#include "temperaments/GOTemperamentList.h"

#include "GOMidiDeviceConfigList.h"
#include "GOOrganList.h"
#include "GOPortsConfig.h"
#include "ptrvector.h"

typedef struct {
  wxString name;
  float left;
  float right;
} GOAudioGroupOutputConfig;

typedef struct {
  wxString name;
  unsigned channels;
  unsigned desired_latency;
  std::vector<std::vector<GOAudioGroupOutputConfig>> scale_factors;
} GOAudioDeviceConfig;

typedef struct {
  GOMidiReceiverType type;
  unsigned index;
  const wxString group;
  const wxString name;
} GOMidiSetting;

enum class GOInitialLoadType { LOAD_NONE, LOAD_LAST_USED, LOAD_FIRST };

class GOConfig : public GOSettingStore, public GOOrganList {
private:
  wxString m_InstanceName;
  wxString m_ConfigFileName;
  wxString m_ResourceDir;
  std::vector<wxString> m_AudioGroups;
  GOPortsConfig m_SoundPortsConfig;
  std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;

  GOPortsConfig m_MidiPortsConfig;
  ptr_vector<GOMidiReceiverBase> m_MIDIEvents;
  GOMidiMap m_MidiMap;
  GOTemperamentList m_Temperaments;

  GOLogicalRect m_MainWindowRect;

  static const GOMidiSetting m_MIDISettings[];
  static const struct IniFileEnumEntry m_InitialLoadTypes[];

  wxString GetEventSection(unsigned index);

public:
  GOConfig(wxString instance);
  ~GOConfig();

  GOSettingDirectory OrganSettingsPath;
  GOSettingDirectory OrganCachePath;

  GOSettingUnsigned Concurrency;
  GOSettingUnsigned ReleaseConcurrency;
  GOSettingUnsigned LoadConcurrency;

  GOSettingUnsigned InterpolationType;
  GOSettingUnsigned WaveFormatBytesPerSample;
  GOSettingBool RecordDownmix;

  GOSettingUnsigned AttackLoad;
  GOSettingUnsigned LoopLoad;
  GOSettingUnsigned ReleaseLoad;

  GOSettingBool ManageCache;
  GOSettingBool CompressCache;
  GOSettingEnum<GOInitialLoadType> LoadLastFile;
  GOSettingBool ODFCheck;
  GOSettingBool ODFHw1Check;

  GOSettingUnsigned LoadChannels;
  GOSettingBool LosslessCompression;
  GOSettingBool ManagePolyphony;
  GOSettingBool ScaleRelease;
  GOSettingBool RandomizeSpeaking;
  GOSettingBool ReverbEnabled;
  GOSettingBool ReverbDirect;
  GOSettingUnsigned ReverbChannel;
  GOSettingUnsigned ReverbStartOffset;
  GOSettingUnsigned ReverbLen;
  GOSettingUnsigned ReverbDelay;
  GOSettingFloat ReverbGain;
  GOSettingFile ReverbFile;

  GOSettingFloat MemoryLimit;
  GOSettingUnsigned SamplesPerBuffer;
  GOSettingUnsigned SampleRate;
  GOSettingInteger Volume;
  GOSettingUnsigned PolyphonyLimit;
  GOSettingUnsigned Preset;
  GOSettingString LanguageCode;

  class GOSettingUnsignedBit : public GOSettingUnsigned {
  protected:
    unsigned validate(unsigned value) {
      return GOSettingUnsigned::validate(value - value % 4);
    }

  public:
    GOSettingUnsignedBit(
      GOSettingStore *store,
      wxString group,
      wxString name,
      unsigned min_value,
      unsigned max_value,
      unsigned default_value)
      : GOSettingUnsigned(
        store, group, name, min_value, max_value, default_value) {}

  } BitsPerSample;

  GOSettingInteger Transpose;

  GOSettingUnsigned MetronomeMeasure;
  GOSettingUnsigned MetronomeBPM;

  GOSettingBool IsToAutoAddMidi;
  GOSettingBool IsToCheckMidiOnStart;
  GOSettingString MidiRecorderOutputDevice;

  GOSettingDirectory OrganPath;
  GOSettingDirectory OrganPackagePath;
  GOSettingDirectory OrganCombinationsPath;
  GOSettingDirectory ExportImportPath;
  GOSettingDirectory AudioRecorderPath;
  GOSettingDirectory MidiRecorderPath;
  GOSettingDirectory MidiPlayerPath;

  GOSettingBool CheckForUpdatesAtStartup;

  GOMidiDeviceConfigList m_MidiIn;
  GOMidiDeviceConfigList m_MidiOut;

  GODialogSizeSet m_DialogSizes;

  void Load();

  int GetLanguageId() const;
  void SetLanguageId(int langId);

  const wxString &GetResourceDirectory() const { return m_ResourceDir; }
  const wxString GetPackageDirectory();

  unsigned GetEventCount() const;
  wxString GetEventGroup(unsigned index);
  wxString GetEventTitle(unsigned index);
  const GOMidiReceiverBase *GetMidiEvent(unsigned index) const;
  const GOMidiReceiverBase *FindMidiEvent(
    GOMidiReceiverType type, unsigned index) const;

  const std::vector<wxString> &GetAudioGroups();
  void SetAudioGroups(const std::vector<wxString> &audio_groups);
  unsigned GetAudioGroupId(const wxString &str) const;
  int GetStrictAudioGroupId(const wxString &str);

  const GOPortsConfig &GetSoundPortsConfig() const {
    return m_SoundPortsConfig;
  }

  void SetSoundPortsConfig(const GOPortsConfig &portsConfig) {
    m_SoundPortsConfig = portsConfig;
  }

  const std::vector<GOAudioDeviceConfig> &GetAudioDeviceConfig();
  const unsigned GetTotalAudioChannels() const;
  void SetAudioDeviceConfig(const std::vector<GOAudioDeviceConfig> &config);
  unsigned GetDefaultLatency();

  const GOPortsConfig &GetMidiPortsConfig() const { return m_MidiPortsConfig; }

  void SetMidiPortsConfig(const GOPortsConfig &portsConfig) {
    m_MidiPortsConfig = portsConfig;
  }

  GOMidiMap &GetMidiMap();

  GOTemperamentList &GetTemperaments();

  const GOLogicalRect &GetMainWindowRect() const { return m_MainWindowRect; }
  void SetMainWindowRect(const GOLogicalRect &rect) { m_MainWindowRect = rect; }

  void Flush();
};

#endif
