/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOConfig.h"

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/thread.h>

#include "GOMemoryPool.h"
#include "GOOrgan.h"
#include "GOPath.h"
#include "GOPortFactory.h"
#include "GOStdPath.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "config/GOConfigWriter.h"
#include "go_limits.h"
#include "midi/ports/GOMidiPort.h"
#include "midi/ports/GOMidiPortFactory.h"
#include "settings/GOSettingEnum.cpp"
#include "settings/GOSettingNumber.cpp"
#include "sound/GOSoundDefs.h"
#include "sound/ports/GOSoundPort.h"
#include "sound/ports/GOSoundPortFactory.h"

static const wxString COUNT = wxT("Count");
static const wxString ENABLED = wxT(".Enabled");
static const wxString MIDI_PORTS = wxT("MidiPorts");
static const wxString MIDI_IN(wxT("MIDIIn"));
static const wxString MIDI_OUT(wxT("MIDIOut"));
static const wxString SOUND_PORTS = wxT("SoundPorts");

const GOMidiSetting GOConfig::m_MIDISettings[] = {
  {MIDI_RECV_MANUAL, 1, wxTRANSLATE("Manuals"), wxTRANSLATE("Pedal")},
  {MIDI_RECV_MANUAL, 2, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 1")},
  {MIDI_RECV_MANUAL, 3, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 2")},
  {MIDI_RECV_MANUAL, 4, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 3")},
  {MIDI_RECV_MANUAL, 5, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 4")},
  {MIDI_RECV_MANUAL, 6, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 5")},
  {MIDI_RECV_ENCLOSURE,
   1,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 1")},
  {MIDI_RECV_ENCLOSURE,
   2,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 2")},
  {MIDI_RECV_ENCLOSURE,
   3,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 3")},
  {MIDI_RECV_ENCLOSURE,
   4,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 4")},
  {MIDI_RECV_ENCLOSURE,
   5,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 5")},
  {MIDI_RECV_ENCLOSURE,
   6,
   wxTRANSLATE("Enclosures"),
   wxTRANSLATE("Enclosure 6")},
  {MIDI_RECV_SETTER,
   0,
   wxTRANSLATE("Sequencer"),
   wxTRANSLATE("Previous Memory")},
  {MIDI_RECV_SETTER, 1, wxTRANSLATE("Sequencer"), wxTRANSLATE("Next Memory")},
  {MIDI_RECV_SETTER, 2, wxTRANSLATE("Sequencer"), wxTRANSLATE("Memory Set")},
  {MIDI_RECV_SETTER, 3, wxTRANSLATE("Sequencer"), wxTRANSLATE("Current")},
  {MIDI_RECV_SETTER, 4, wxTRANSLATE("Sequencer"), wxTRANSLATE("G.C.")},
  {MIDI_RECV_SETTER, 5, wxTRANSLATE("Sequencer"), wxTRANSLATE("-10")},
  {MIDI_RECV_SETTER, 6, wxTRANSLATE("Sequencer"), wxTRANSLATE("+10")},
  {MIDI_RECV_SETTER, 7, wxTRANSLATE("Sequencer"), wxTRANSLATE("__0")},
  {MIDI_RECV_SETTER, 8, wxTRANSLATE("Sequencer"), wxTRANSLATE("__1")},
  {MIDI_RECV_SETTER, 9, wxTRANSLATE("Sequencer"), wxTRANSLATE("__2")},
  {MIDI_RECV_SETTER, 10, wxTRANSLATE("Sequencer"), wxTRANSLATE("__3")},
  {MIDI_RECV_SETTER, 11, wxTRANSLATE("Sequencer"), wxTRANSLATE("__4")},
  {MIDI_RECV_SETTER, 12, wxTRANSLATE("Sequencer"), wxTRANSLATE("__5")},
  {MIDI_RECV_SETTER, 13, wxTRANSLATE("Sequencer"), wxTRANSLATE("__6")},
  {MIDI_RECV_SETTER, 14, wxTRANSLATE("Sequencer"), wxTRANSLATE("__7")},
  {MIDI_RECV_SETTER, 15, wxTRANSLATE("Sequencer"), wxTRANSLATE("__8")},
  {MIDI_RECV_SETTER, 16, wxTRANSLATE("Sequencer"), wxTRANSLATE("__9")},
  {MIDI_RECV_SETTER,
   17,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("-1 Cent")},
  {MIDI_RECV_SETTER,
   18,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("+1 Cent")},
  {MIDI_RECV_SETTER,
   19,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("-100 Cent")},
  {MIDI_RECV_SETTER,
   20,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("+100 Cent")},
  {MIDI_RECV_SETTER,
   21,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("Prev temperament")},
  {MIDI_RECV_SETTER,
   22,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("Next temperament")},
  {MIDI_RECV_SETTER,
   23,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("Transpose -")},
  {MIDI_RECV_SETTER,
   24,
   wxTRANSLATE("Master Controls"),
   wxTRANSLATE("Transpose +")},
  {MIDI_RECV_SETTER, 25, wxTRANSLATE("Metronome"), wxTRANSLATE("On")},
  {MIDI_RECV_SETTER, 26, wxTRANSLATE("Metronome"), wxTRANSLATE("BPM +")},
  {MIDI_RECV_SETTER, 27, wxTRANSLATE("Metronome"), wxTRANSLATE("BPM -")},
  {MIDI_RECV_SETTER, 28, wxTRANSLATE("Metronome"), wxTRANSLATE("Measure -")},
  {MIDI_RECV_SETTER, 29, wxTRANSLATE("Metronome"), wxTRANSLATE("Measure +")},
};

const struct IniFileEnumEntry GOConfig::m_InitialLoadTypes[] = {
  {wxT("N"), (int)GOInitialLoadType::LOAD_NONE},
  {wxT("Y"), (int)GOInitialLoadType::LOAD_LAST_USED},
  {wxT("First"), (int)GOInitialLoadType::LOAD_FIRST},
};

GOConfig::GOConfig(wxString instance)
  : m_InstanceName(instance),
    m_ResourceDir(),
    m_AudioGroups(),
    m_AudioDeviceConfig(),
    m_MIDIEvents(),
    OrganSettingsPath(this, wxT("General"), wxT("SettingPath"), wxEmptyString),
    OrganCachePath(this, wxT("General"), wxT("CachePath"), wxEmptyString),
    Concurrency(this, wxT("General"), wxT("Concurrency"), 0, MAX_CPU, 1),
    ReleaseConcurrency(
      this, wxT("General"), wxT("ReleaseConcurrency"), 1, MAX_CPU, 1),
    LoadConcurrency(
      this, wxT("General"), wxT("LoadConcurrency"), 0, MAX_CPU, 1),
    InterpolationType(this, wxT("General"), wxT("InterpolationType"), 0, 1, 0),
    WaveFormatBytesPerSample(this, wxT("General"), wxT("WaveFormat"), 1, 4, 4),
    RecordDownmix(this, wxT("General"), wxT("RecordDownmix"), false),
    AttackLoad(this, wxT("General"), wxT("AttackLoad"), 0, 1, 1),
    LoopLoad(this, wxT("General"), wxT("LoopLoad"), 0, 2, 2),
    ReleaseLoad(this, wxT("General"), wxT("ReleaseLoad"), 0, 1, 1),
    ManageCache(this, wxT("General"), wxT("ManageCache"), true),
    CompressCache(this, wxT("General"), wxT("CompressCache"), false),
    LoadLastFile(
      this,
      wxT("General"),
      wxT("LoadLastFile"),
      m_InitialLoadTypes,
      sizeof(m_InitialLoadTypes) / sizeof(m_InitialLoadTypes[0]),
      GOInitialLoadType::LOAD_LAST_USED),
    ODFCheck(this, wxT("General"), wxT("StrictODFCheck"), false),
    ODFHw1Check(this, wxT("General"), wxT("ODFHw1Check"), false),
    LoadChannels(this, wxT("General"), wxT("Channels"), 0, 2, 2),
    LosslessCompression(
      this, wxT("General"), wxT("LosslessCompression"), false),
    ManagePolyphony(this, wxT("General"), wxT("ManagePolyphony"), true),
    ScaleRelease(this, wxT("General"), wxT("ScaleRelease"), true),
    RandomizeSpeaking(this, wxT("General"), wxT("RandomizeSpeaking"), true),
    ReverbEnabled(this, wxT("Reverb"), wxT("ReverbEnabled"), false),
    ReverbDirect(this, wxT("Reverb"), wxT("ReverbDirect"), true),
    ReverbChannel(this, wxT("Reverb"), wxT("ReverbChannel"), 1, 4, 1),
    ReverbStartOffset(
      this, wxT("Reverb"), wxT("ReverbStartOffset"), 0, MAX_SAMPLE_LENGTH, 0),
    ReverbLen(this, wxT("Reverb"), wxT("ReverbLen"), 0, MAX_SAMPLE_LENGTH, 0),
    ReverbDelay(this, wxT("Reverb"), wxT("ReverbDelay"), 0, 10000, 0),
    ReverbGain(this, wxT("Reverb"), wxT("ReverbGain"), 0, 50, 1),
    ReverbFile(this, wxT("Reverb"), wxT("ReverbFile"), wxEmptyString),
    MemoryLimit(
      this,
      wxT("General"),
      wxT("MemoryLimit"),
      0,
      1024 * 1024,
      GOMemoryPool::GetSystemMemoryLimit()),
    SamplesPerBuffer(
      this, wxT("General"), wxT("SamplesPerBuffer"), 1, MAX_FRAME_SIZE, 1024),
    SampleRate(this, wxT("General"), wxT("SampleRate"), 1000, 100000, 44100),
    Volume(this, wxT("General"), wxT("Volume"), -120, 20, -15),
    PolyphonyLimit(
      this, wxT("General"), wxT("PolyphonyLimit"), 0, MAX_POLYPHONY, 2048),
    Preset(this, wxT("General"), wxT("Preset"), 0, MAX_PRESET, 0),
    LanguageCode(this, wxT("General"), wxT("Language"), wxEmptyString),
    BitsPerSample(this, wxT("General"), wxT("BitsPerSample"), 8, 24, 24),
    Transpose(this, wxT("General"), wxT("Transpose"), -11, 11, 0),
    MetronomeMeasure(this, wxT("Metronome"), wxT("Measure"), 0, 32, 4),
    MetronomeBPM(this, wxT("Metronome"), wxT("BPM"), 1, 500, 80),
    IsToAutoAddMidi(this, MIDI_IN, wxT("IsToAutoAddMidi"), true),
    IsToCheckMidiOnStart(this, MIDI_IN, wxT("IsToCheckMidiOnStart"), true),
    MidiRecorderOutputDevice(
      this, MIDI_OUT, wxT("MIDIRecorderDevice"), wxEmptyString),
    OrganPath(this, wxT("General"), wxT("OrganPath"), wxEmptyString),
    OrganPackagePath(
      this, wxT("General"), wxT("OrganPackagePath"), wxEmptyString),
    OrganCombinationsPath(
      this, wxT("General"), wxT("CmbYamlPath"), wxEmptyString),
    ExportImportPath(this, wxT("General"), wxT("CMBPath"), wxEmptyString),
    AudioRecorderPath(
      this, wxT("General"), wxT("AudioRecorder"), wxEmptyString),
    MidiRecorderPath(
      this, wxT("General"), wxT("MIDIRecorderPath"), wxEmptyString),
    MidiPlayerPath(this, wxT("General"), wxT("MIDIPlayerPath"), wxEmptyString),
    CheckForUpdatesAtStartup(
      this, wxT("General"), wxT("CheckForUpdatesAtStartup"), true),
    m_MidiIn(MIDI_IN),
    m_MidiOut(MIDI_OUT) {
  m_ConfigFileName = GOStdPath::GetConfigDir() + wxFileName::GetPathSeparator()
    + wxT("GrandOrgueConfig") + m_InstanceName;
  for (unsigned i = 0; i < GetEventCount(); i++)
    m_MIDIEvents.push_back(new GOMidiReceiverBase(m_MIDISettings[i].type));
  m_ResourceDir = GOStdPath::GetResourceDir();

  OrganPath.setDefaultValue(GOStdPath::GetGrandOrgueSubDir(_("Organs")));
  OrganPackagePath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Organ packages")));
  OrganCachePath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(wxT("Cache") + m_InstanceName));
  OrganSettingsPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(wxT("Data") + m_InstanceName));
  OrganCombinationsPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Combinations")));
  ExportImportPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Settings")));
  AudioRecorderPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Audio recordings")));
  MidiRecorderPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("MIDI recordings")));
  MidiPlayerPath.setDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("MIDI recordings")));
}

GOConfig::~GOConfig() { /* Flush(); */
}

void load_ports_config(
  GOConfigReader &cfg,
  const wxString &groupName,
  const GOPortFactory &factory,
  GOPortsConfig &portsConfig) {
  portsConfig.Clear();
  for (const wxString &portName : factory.GetPortNames()) {
    const bool isPortEnabled
      = cfg.ReadBoolean(CMBSetting, groupName, portName + ENABLED, false, true);
    const wxString prefix = portName + ".";

    portsConfig.SetConfigEnabled(portName, isPortEnabled);

    for (const wxString &apiName : factory.GetPortApiNames(portName))
      portsConfig.SetConfigEnabled(
        portName,
        apiName,
        cfg.ReadBoolean(
          CMBSetting, groupName, prefix + apiName + ENABLED, false, true));
  }
}

void save_ports_config(
  GOConfigWriter &cfg,
  const wxString &groupName,
  const GOPortFactory &factory,
  const GOPortsConfig &portsConfig) {
  for (const wxString &portName : factory.GetPortNames()) {
    cfg.WriteBoolean(
      groupName, portName + ENABLED, portsConfig.IsConfigEnabled(portName));
    const wxString prefix = portName + ".";
    for (const wxString &apiName : factory.GetPortApiNames(portName))
      cfg.WriteBoolean(
        groupName,
        prefix + apiName + ENABLED,
        portsConfig.IsConfigEnabled(portName, apiName));
  }
}

void GOConfig::Load() {
  GOConfigFileReader cfg_file;
  if (wxFileExists(m_ConfigFileName)) {
    if (!cfg_file.Read(m_ConfigFileName))
      wxLogError(_("Unable to read '%s'"), m_ConfigFileName.c_str());
  } else {
    wxString fileName = wxStandardPaths::Get().GetUserDataDir()
      + wxFileName::GetPathSeparator() + wxT("GrandOrgueConfig")
      + m_InstanceName;
    if (wxFileExists(fileName))
      if (!cfg_file.Read(fileName))
        wxLogError(_("Unable to read '%s'"), fileName.c_str());
  }

  try {
    GOConfigReaderDB cfg_db;
    cfg_db.ReadData(cfg_file, CMBSetting, false);
    GOConfigReader cfg(cfg_db);

    GOOrganList::Load(cfg, m_MidiMap);

    m_MainWindowRect.x = cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowX"), -32000, 32000, false, 0);
    m_MainWindowRect.y = cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowY"), -32000, 32000, false, 0);
    m_MainWindowRect.width = (unsigned)cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowWidth"), 0, 32000, false, 0);
    m_MainWindowRect.height = (unsigned)cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowHeight"), 0, 32000, false, 0);

    m_Temperaments.InitTemperaments();
    m_Temperaments.Load(cfg);

    m_AudioGroups.clear();
    unsigned count = cfg.ReadInteger(
      CMBSetting, wxT("AudioGroups"), COUNT, 0, 200, false, 0);
    for (unsigned i = 0; i < count; i++)
      m_AudioGroups.push_back(cfg.ReadString(
        CMBSetting,
        wxT("AudioGroups"),
        wxString::Format(wxT("Name%03d"), i + 1),
        false,
        wxString::Format(_("Audio group %d"), i + 1)));
    if (!m_AudioGroups.size())
      m_AudioGroups.push_back(_("Default audio group"));

    load_ports_config(
      cfg, SOUND_PORTS, GOSoundPortFactory::getInstance(), m_SoundPortsConfig);

    m_AudioDeviceConfig.clear();
    count = cfg.ReadInteger(
      CMBSetting, wxT("AudioDevices"), COUNT, 0, 200, false, 0);
    for (unsigned i = 0; i < count; i++) {
      GOAudioDeviceConfig conf;
      conf.name = cfg.ReadString(
        CMBSetting,
        wxT("AudioDevices"),
        wxString::Format(wxT("Device%03dName"), i + 1));
      conf.channels = cfg.ReadInteger(
        CMBSetting,
        wxT("AudioDevices"),
        wxString::Format(wxT("Device%03dChannelCount"), i + 1),
        0,
        200);
      conf.desired_latency = cfg.ReadInteger(
        CMBSetting,
        wxT("AudioDevices"),
        wxString::Format(wxT("Device%03dLatency"), i + 1),
        0,
        999,
        false,
        GetDefaultLatency());
      conf.scale_factors.resize(conf.channels);
      for (unsigned j = 0; j < conf.channels; j++) {
        wxString prefix
          = wxString::Format(wxT("Device%03dChannel%03d"), i + 1, j + 1);
        unsigned group_count = cfg.ReadInteger(
          CMBSetting, wxT("AudioDevices"), prefix + wxT("GroupCount"), 0, 200);
        for (unsigned k = 0; k < group_count; k++) {
          GOAudioGroupOutputConfig group;
          wxString p = prefix + wxString::Format(wxT("Group%03d"), k + 1);

          group.name
            = cfg.ReadString(CMBSetting, wxT("AudioDevices"), p + wxT("Name"));
          group.left = cfg.ReadFloat(
            CMBSetting, wxT("AudioDevices"), p + wxT("Left"), -121.0, 40);
          group.right = cfg.ReadFloat(
            CMBSetting, wxT("AudioDevices"), p + wxT("Right"), -121.0, 40);

          conf.scale_factors[j].push_back(group);
        }
      }
      m_AudioDeviceConfig.push_back(conf);
    }

    load_ports_config(
      cfg, MIDI_PORTS, GOMidiPortFactory::getInstance(), m_MidiPortsConfig);

    for (unsigned i = 0; i < GetEventCount(); i++)
      m_MIDIEvents[i]->Load(cfg, GetEventSection(i), m_MidiMap);

    long cpus = wxThread::GetCPUCount();
    if (cpus == -1)
      cpus = 4;
    if (cpus > MAX_CPU)
      cpus = MAX_CPU;
    if (cpus == 0)
      cpus = 1;

    Concurrency.setDefaultValue(cpus);
    ReleaseConcurrency.setDefaultValue(cpus);
    LoadConcurrency.setDefaultValue(cpus);
    PolyphonyLimit.setDefaultValue(cpus * 725),

      GOSettingStore::Load(cfg);

    // Fill ODFHw1Check with ODFCheck by default
    if (!ODFHw1Check.IsPresent())
      ODFHw1Check(ODFCheck());

    if (Concurrency() == 0)
      Concurrency(1);

    m_MidiOut.Load(cfg);
    m_MidiIn.Load(cfg, &m_MidiOut);

    m_DialogSizes.Load(cfg, CMBSetting);

    if (wxFileExists(m_ConfigFileName))
      wxCopyFile(m_ConfigFileName, m_ConfigFileName + wxT(".last"));
  } catch (wxString error) {
    wxLogError(wxT("%s\n"), error.c_str());
  }

  if (!m_AudioDeviceConfig.size()) {
    GOAudioDeviceConfig conf;
    conf.name = wxEmptyString;
    conf.channels = 2;
    conf.scale_factors.resize(conf.channels);
    conf.desired_latency = GetDefaultLatency();
    for (unsigned k = 0; k < m_AudioGroups.size(); k++) {
      GOAudioGroupOutputConfig group;
      group.name = m_AudioGroups[k];

      group.left = 0.0f;
      group.right = -121.0f;
      conf.scale_factors[0].push_back(group);

      group.left = -121.0f;
      group.right = 0.0f;
      conf.scale_factors[1].push_back(group);
    }
    m_AudioDeviceConfig.push_back(conf);
  }
}

int GOConfig::GetLanguageId() const {
  int langId = wxLANGUAGE_DEFAULT;
  const wxString langCode = LanguageCode();

  if (!langCode.IsEmpty()) {
    const wxLanguageInfo *const langInfo = wxLocale::FindLanguageInfo(langCode);

    if (langInfo != NULL)
      langId = langInfo->Language;
  }
  return langId;
}

void GOConfig::SetLanguageId(int langId) {
  LanguageCode(
    langId == wxLANGUAGE_DEFAULT ? wxString(wxEmptyString)
                                 : wxLocale::GetLanguageCanonicalName(langId));
}

unsigned GOConfig::GetEventCount() const {
  return sizeof(m_MIDISettings) / sizeof(m_MIDISettings[0]);
}

wxString GOConfig::GetEventSection(unsigned index) {
  assert(index < GetEventCount());
  switch (m_MIDISettings[index].type) {
  case MIDI_RECV_ENCLOSURE:
    return wxString::Format(wxT("Enclosure%03d"), m_MIDISettings[index].index);

  case MIDI_RECV_MANUAL:
    return wxString::Format(wxT("Manual%03d"), m_MIDISettings[index].index);

  case MIDI_RECV_SETTER:
    return wxString::Format(wxT("Setter%03d"), m_MIDISettings[index].index);

  default:
    assert(false);
    return wxEmptyString;
  }
}

wxString GOConfig::GetEventGroup(unsigned index) {
  assert(index < GetEventCount());
  return wxGetTranslation(m_MIDISettings[index].group);
}

wxString GOConfig::GetEventTitle(unsigned index) {
  assert(index < GetEventCount());
  return wxGetTranslation(m_MIDISettings[index].name);
}

const GOMidiReceiverBase *GOConfig::GetMidiEvent(unsigned index) const {
  assert(index < GetEventCount());
  return m_MIDIEvents[index];
}

const GOMidiReceiverBase *GOConfig::FindMidiEvent(
  GOMidiReceiverType type, unsigned index) const {
  for (unsigned i = 0; i < GetEventCount(); i++)
    if (m_MIDISettings[i].type == type && m_MIDISettings[i].index == index)
      return m_MIDIEvents[i];
  return NULL;
}

const wxString GOConfig::GetPackageDirectory() {
  return m_ResourceDir + wxFileName::GetPathSeparator() + wxT("packages");
}

/*
bool GOSettings::GetMidiInState(wxString device, bool isEnabledByDefault)
{
        bool isEnabled = false;
        std::map<wxString, bool>::iterator it = m_MidiIn.find(device);

        if (it != m_MidiIn.end())
                isEnabled = it->second;
        else
        {
                isEnabled
                  = isEnabledByDefault && device.Find(wxT("GrandOrgue")) ==
wxNOT_FOUND; m_MidiIn[device] = isEnabled;
        }
        return isEnabled;
}

void GOSettings::SetMidiInState(wxString device, bool enabled)
{
        m_MidiIn[device] = enabled;
}

unsigned GOSettings::GetMidiInDeviceChannelShift(wxString device)
{
        std::map<wxString, unsigned>::iterator it = m_MidiInShift.find(device);
        if (it == m_MidiInShift.end())
                return 0;
        else
                return it->second;
}

void GOSettings::SetMidiInDeviceChannelShift(wxString device, unsigned shift)
{
        shift = shift % 16;
        m_MidiInShift[device] = shift;
}

wxString GOSettings::GetMidiInOutDevice(wxString device)
{
        std::map<wxString, wxString>::iterator it =
m_MidiInOutDeviceMap.find(device); if (it == m_MidiInOutDeviceMap.end()) return
wxEmptyString; else return it->second;
}

void GOSettings::SetMidiInOutDevice(wxString device, wxString out_device)
{
        m_MidiInOutDeviceMap[device] = out_device;
}

std::vector<wxString> GOSettings::GetMidiInDeviceList()
{
        std::vector<wxString> list;
        for (std::map<wxString, bool>::iterator it = m_MidiIn.begin(); it !=
m_MidiIn.end(); it++) list.push_back(it->first); return list;
}

bool GOSettings::GetMidiOutState(wxString device)
{
        std::map<wxString, bool>::iterator it = m_MidiOut.find(device);
        if (it == m_MidiOut.end())
                return false;
        else
                return it->second;
}

void GOSettings::SetMidiOutState(wxString device, bool enabled)
{
        m_MidiOut[device] = enabled;
}

std::vector<wxString> GOSettings::GetMidiOutDeviceList()
{
        std::vector<wxString> list;
        for (std::map<wxString, bool>::iterator it = m_MidiOut.begin(); it !=
m_MidiOut.end(); it++) list.push_back(it->first); return list;
}
 */

const std::vector<wxString> &GOConfig::GetAudioGroups() {
  return m_AudioGroups;
}

void GOConfig::SetAudioGroups(const std::vector<wxString> &audio_groups) {
  if (!audio_groups.size())
    return;
  m_AudioGroups = audio_groups;
}

unsigned GOConfig::GetAudioGroupId(const wxString &str) const {
  for (unsigned i = 0; i < m_AudioGroups.size(); i++)
    if (m_AudioGroups[i] == str)
      return i;
  return 0;
}

int GOConfig::GetStrictAudioGroupId(const wxString &str) {
  for (unsigned i = 0; i < m_AudioGroups.size(); i++)
    if (m_AudioGroups[i] == str)
      return i;
  return -1;
}

const std::vector<GOAudioDeviceConfig> &GOConfig::GetAudioDeviceConfig() {
  return m_AudioDeviceConfig;
}

void GOConfig::SetAudioDeviceConfig(
  const std::vector<GOAudioDeviceConfig> &config) {
  if (!config.size())
    return;
  m_AudioDeviceConfig = config;
}

const unsigned GOConfig::GetTotalAudioChannels() const {
  unsigned channels = 0;

  for (const GOAudioDeviceConfig &deviceConfig : m_AudioDeviceConfig)
    channels += deviceConfig.channels;
  return channels;
}

unsigned GOConfig::GetDefaultLatency() { return 50; }

GOMidiMap &GOConfig::GetMidiMap() { return m_MidiMap; }

GOTemperamentList &GOConfig::GetTemperaments() { return m_Temperaments; }

void GOConfig::Flush() {
  wxString tmp_name = m_ConfigFileName + wxT(".new");
  GOConfigFileWriter cfg_file;
  GOConfigWriter cfg(cfg_file, false);

  GOSettingStore::Save(cfg);
  GOOrganList::Save(cfg, m_MidiMap);

  cfg.WriteInteger(wxT("UI"), wxT("MainWindowX"), m_MainWindowRect.x);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowY"), m_MainWindowRect.y);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowWidth"), m_MainWindowRect.width);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowHeight"), m_MainWindowRect.height);

  m_Temperaments.Save(cfg);

  for (unsigned i = 0; i < GetEventCount(); i++)
    m_MIDIEvents[i]->Save(cfg, GetEventSection(i), m_MidiMap);

  for (unsigned i = 0; i < m_AudioGroups.size(); i++)
    cfg.WriteString(
      wxT("AudioGroups"),
      wxString::Format(wxT("Name%03d"), i + 1),
      m_AudioGroups[i]);
  cfg.WriteInteger(wxT("AudioGroups"), COUNT, m_AudioGroups.size());

  save_ports_config(
    cfg, SOUND_PORTS, GOSoundPortFactory::getInstance(), m_SoundPortsConfig);

  for (unsigned i = 0; i < m_AudioDeviceConfig.size(); i++) {
    cfg.WriteString(
      wxT("AudioDevices"),
      wxString::Format(wxT("Device%03dName"), i + 1),
      m_AudioDeviceConfig[i].name);
    cfg.WriteInteger(
      wxT("AudioDevices"),
      wxString::Format(wxT("Device%03dChannelCount"), i + 1),
      m_AudioDeviceConfig[i].channels);
    cfg.WriteInteger(
      wxT("AudioDevices"),
      wxString::Format(wxT("Device%03dLatency"), i + 1),
      m_AudioDeviceConfig[i].desired_latency);
    for (unsigned j = 0; j < m_AudioDeviceConfig[i].channels; j++) {
      wxString prefix
        = wxString::Format(wxT("Device%03dChannel%03d"), i + 1, j + 1);
      cfg.WriteInteger(
        wxT("AudioDevices"),
        prefix + wxT("GroupCount"),
        m_AudioDeviceConfig[i].scale_factors[j].size());
      for (unsigned k = 0; k < m_AudioDeviceConfig[i].scale_factors[j].size();
           k++) {
        wxString p = prefix + wxString::Format(wxT("Group%03d"), k + 1);
        cfg.WriteString(
          wxT("AudioDevices"),
          p + wxT("Name"),
          m_AudioDeviceConfig[i].scale_factors[j][k].name);
        cfg.WriteFloat(
          wxT("AudioDevices"),
          p + wxT("Left"),
          m_AudioDeviceConfig[i].scale_factors[j][k].left);
        cfg.WriteFloat(
          wxT("AudioDevices"),
          p + wxT("Right"),
          m_AudioDeviceConfig[i].scale_factors[j][k].right);
      }
    }
  }
  cfg.WriteInteger(wxT("AudioDevices"), COUNT, m_AudioDeviceConfig.size());

  save_ports_config(
    cfg, MIDI_PORTS, GOMidiPortFactory::getInstance(), m_MidiPortsConfig);

  m_MidiOut.Save(cfg, false);
  m_MidiIn.Save(cfg, true);

  m_DialogSizes.Save(cfg);

  if (::wxFileExists(tmp_name) && !::wxRemoveFile(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
    return;
  }
  if (!cfg_file.Save(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
    return;
  }
  if (!GORenameFile(tmp_name, m_ConfigFileName))
    return;
}
