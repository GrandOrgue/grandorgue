/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIG_H
#define GOCONFIG_H

#include <wx/gdicmn.h>
#include <wx/string.h>

#include <unordered_map>
#include <vector>

#include "gui/dialogs/common/GODialogSizeSet.h"
#include "gui/size/GOLogicalRect.h"
#include "midi/GOMidiMap.h"
#include "midi/events/GOMidiReceiverType.h"
#include "settings/GOSettingBool.h"
#include "settings/GOSettingDirectory.h"
#include "settings/GOSettingEnum.h"
#include "settings/GOSettingFile.h"
#include "settings/GOSettingFloat.h"
#include "settings/GOSettingNumber.h"
#include "settings/GOSettingStore.h"
#include "settings/GOSettingString.h"
#include "temperaments/GOTemperamentList.h"

#include "GOAudioDeviceConfig.h"
#include "GOConfigMidiObject.h"
#include "GOMidiDeviceConfigList.h"
#include "GOOrganList.h"
#include "GOPortsConfig.h"
#include "ptrvector.h"

enum class GOInitialLoadType { LOAD_NONE, LOAD_LAST_USED, LOAD_FIRST };

class GOMidiReceiver;

class GOConfig : public GOSettingStore, public GOOrganList {
public:
  enum InterpolationType {
    INTERPOLATION_LINEAR = 0,
    INTERPOLATION_POLYPHASE,
  };

private:
  wxString m_InstanceName;
  wxString m_ConfigFileName;
  wxString m_ResourceDir;
  std::vector<wxString> m_AudioGroups;
  GOPortsConfig m_SoundPortsConfig;
  std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;

  GOPortsConfig m_MidiPortsConfig;

  /* Thre are two sets of initial MIDI objects: built-in and user-added.
   * The built-in MIDI objects are matched by ReceiverType and MIDIInputNumber.
   * The user-added MIDI objects are matched by path.
   * In this vector the first indices correspond the built-in objects and the
   * rest indices correspond the user-added objects.
   */
  ptr_vector<GOConfigMidiObject> m_InitialMidiObjects;
  // Used for finding a user-added Initial MIDI object
  std::
    unordered_map<wxString, GOConfigMidiObject *, wxStringHash, wxStringEqual>
      m_InitialMidiObjectsByPath;

  GOMidiMap m_MidiMap;
  GOTemperamentList m_Temperaments;

  GOLogicalRect m_MainWindowRect;

  GOOrgan *CloneOrgan(const GOOrgan &newOrgan) const override;
  bool IsValidOrgan(const GOOrgan *pOrgan) const override;

  void LoadOrgans(GOConfigReader &cfg);
  void SaveOrgans(GOConfigWriter &cfg);

  void LoadDefaults();

public:
  GOConfig(wxString instance);

  GOSettingDirectory OrganSettingsPath;
  GOSettingDirectory OrganCachePath;

  GOSettingUnsigned Concurrency;
  GOSettingUnsigned ReleaseConcurrency;
  GOSettingUnsigned LoadConcurrency;

  GOSettingUnsigned m_InterpolationType;
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
  GOSettingBool NewBasMelBehaviour;
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

  // return count of built-in initial MIDI objects
  static unsigned getMidiBuiltinCount();

  // return count of all initial MIDI objects, both built-in and user-addeds
  unsigned GetMidiInitialCount() const { return m_InitialMidiObjects.size(); }
  GOConfigMidiObject *GetMidiInitialObject(unsigned index);
  // search among built-in MIDI objects
  GOConfigMidiObject *FindMidiInitialObject(
    GOMidiObject::ObjectType type, unsigned index);
  // search among user-added MIDI objects
  GOConfigMidiObject *FindMidiInitialObject(const wxString &path);
  void AssignToInitial(const GOMidiObject &pObjFrom);
  void DelMidiInitial(unsigned index);

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

  std::vector<GOAudioDeviceConfig> &GetAudioDeviceConfig() {
    return m_AudioDeviceConfig;
  }
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
