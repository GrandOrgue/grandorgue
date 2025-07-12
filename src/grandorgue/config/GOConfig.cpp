/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOConfig.h"

#include <algorithm>
#include <functional>

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/thread.h>

#include "archive/GOArchiveFile.h"
#include "combinations/GOSetter.h"
#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "config/GOConfigWriter.h"
#include "control/GOCallbackButtonControl.h"
#include "control/GOElementCreator.h"
#include "control/GOPushbuttonControl.h"
#include "midi/ports/GOMidiPort.h"
#include "midi/ports/GOMidiPortFactory.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "settings/GOSettingEnum.cpp"
#include "settings/GOSettingNumber.cpp"
#include "sound/GOSoundDefs.h"
#include "sound/ports/GOSoundPort.h"
#include "sound/ports/GOSoundPortFactory.h"

#include "GOConfigMidiObject.h"
#include "GOMemoryPool.h"
#include "GOMetronome.h"
#include "GOPortFactory.h"
#include "GORegisteredOrgan.h"
#include "GOStdPath.h"
#include "go_limits.h"
#include "go_path.h"

static constexpr int MIDI_ELEMENT_NONE = -1;

static constexpr unsigned SAMPLE_RATE_DEFAULT = 48000;
static constexpr unsigned SAMPLES_PER_BUFFER_DEFAULT = 512;
static constexpr GOConfig::InterpolationType INTERPOLATION_DEFAULT
  = GOConfig::INTERPOLATION_POLYPHASE;

static const wxString WX_EMPTY = wxEmptyString;
static const wxString COUNT = wxT("Count");
static const wxString ENABLED = wxT(".Enabled");
static const wxString MIDI_PORTS = wxT("MidiPorts");
static const wxString MIDI_IN(wxT("MIDIIn"));
static const wxString MIDI_OUT(wxT("MIDIOut"));
static const wxString SOUND_PORTS = wxT("SoundPorts");
static const wxString GENERAL = wxT("General");
static const wxString WX_USER_ADDED = wxT("User-added");
static const wxString WX_MIDI_INITIAL_COUNT = wxT("MidiInitialCount");
static const wxString WX_MIDI_INITIAL_FMT = wxT("MidiInitial%03u");
static const wxString WX_MIDI_INPUT_NUMBER = wxT("MidiInputNumber");
static const wxString WX_PATH = wxT("Path");
static const wxString WX_OBJECT_TYPE = wxT("ObjectType");
static const wxString WX_NAME = wxT("Name");
static const wxString WX_FMT_D = wxT("%d");

struct initial_midi_group_desc {
  GOMidiObject::ObjectType m_ObjectType;
  int m_SenderType;
  int m_ReceiverType;
  int m_ShortcutReceiverType;
  int m_DivisionalType;
  wxString m_GroupName;
  wxString m_SectionFmt;
};

enum initial_midi_group {
  INITIAL_MANUAL,
  INITIAL_ENCLOSURE,
  INITIAL_SETTER,
  INITIAL_MASTER,
  INITIAL_METRONOME,
};

// Indexed by initial_midi_group
static const initial_midi_group_desc INITIAL_MIDI_GROUP_DESCS[]{
  {
    GOMidiObject::OBJECT_TYPE_MANUAL,
    MIDI_SEND_MANUAL,
    MIDI_RECV_MANUAL,
    GOConfigMidiObject::ELEMENT_TYPE_NONE,
    MIDI_SEND_MANUAL,
    _("Manuals"),
    wxT("Manual%03d"),
  },
  {
    GOMidiObject::OBJECT_TYPE_ENCLOSURE,
    MIDI_SEND_ENCLOSURE,
    MIDI_RECV_ENCLOSURE,
    KEY_RECV_ENCLOSURE,
    GOConfigMidiObject::ELEMENT_TYPE_NONE,
    _("Enclosures"),
    wxT("Enclosure%03d"),
  },
  {GOMidiObject::OBJECT_TYPE_BUTTON,
   MIDI_SEND_BUTTON,
   MIDI_RECV_SETTER,
   KEY_RECV_BUTTON,
   GOMidiObject::ELEMENT_TYPE_NONE,
   _("Sequencer"),
   wxT("Setter%03d")},
  {GOMidiObject::OBJECT_TYPE_BUTTON,
   MIDI_SEND_BUTTON,
   MIDI_RECV_SETTER,
   KEY_RECV_BUTTON,
   GOMidiObject::ELEMENT_TYPE_NONE,
   _("Master Controls"),
   wxT("Setter%03d")},
  {GOMidiObject::OBJECT_TYPE_BUTTON,
   MIDI_SEND_BUTTON,
   MIDI_RECV_SETTER,
   KEY_RECV_BUTTON,
   GOMidiObject::ELEMENT_TYPE_NONE,
   _("Metronome"),
   wxT("Setter%03d")}};

struct internal_midi_object_desc {
  initial_midi_group m_group;
  unsigned m_index;
  wxString m_name;
  const GOElementCreator::ButtonDefinitionEntry *p_ButtonDef;
};

static const internal_midi_object_desc INTERNAL_MIDI_DESCS[] = {
  {INITIAL_MANUAL, 1, _("Pedal"), nullptr},
  {INITIAL_MANUAL, 2, _("Manual 1"), nullptr},
  {INITIAL_MANUAL, 3, _("Manual 2"), nullptr},
  {INITIAL_MANUAL, 4, _("Manual 3"), nullptr},
  {INITIAL_MANUAL, 5, _("Manual 4"), nullptr},
  {INITIAL_MANUAL, 6, _("Manual 5"), nullptr},
  {INITIAL_ENCLOSURE, 1, _("Enclosure 1"), nullptr},
  {INITIAL_ENCLOSURE, 2, _("Enclosure 2"), nullptr},
  {INITIAL_ENCLOSURE, 3, _("Enclosure 3"), nullptr},
  {INITIAL_ENCLOSURE, 4, _("Enclosure 4"), nullptr},
  {INITIAL_ENCLOSURE, 5, _("Enclosure 5"), nullptr},
  {INITIAL_ENCLOSURE, 6, _("Enclosure 6"), nullptr},
  {INITIAL_SETTER,
   0,
   _("Previous Memory"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_PREV},
  {INITIAL_SETTER,
   1,
   _("Next Memory"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_NEXT},
  {INITIAL_SETTER,
   2,
   _("Memory Set"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_SET},
  {INITIAL_SETTER,
   3,
   _("Current"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_CURRENT},
  {INITIAL_SETTER,
   4,
   _("G.C."),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_GC},
  {INITIAL_SETTER,
   5,
   _("-10"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_M10},
  {INITIAL_SETTER,
   6,
   _("+10"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_P10},
  {INITIAL_SETTER,
   7,
   _("__0"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L0},
  {INITIAL_SETTER,
   8,
   _("__1"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L1},
  {INITIAL_SETTER,
   9,
   _("__2"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L2},
  {INITIAL_SETTER,
   10,
   _("__3"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L3},
  {INITIAL_SETTER,
   11,
   _("__4"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L4},
  {INITIAL_SETTER,
   12,
   _("__5"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L5},
  {INITIAL_SETTER,
   13,
   _("__6"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L6},
  {INITIAL_SETTER,
   14,
   _("__7"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L7},
  {INITIAL_SETTER,
   15,
   _("__8"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L8},
  {INITIAL_SETTER,
   16,
   _("__9"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_L9},
  {INITIAL_MASTER,
   17,
   _("-1 Cent"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_PITCH_M1},
  {INITIAL_MASTER,
   18,
   _("+1 Cent"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_PITCH_P1},
  {INITIAL_MASTER,
   19,
   _("-100 Cent"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_PITCH_M100},
  {INITIAL_MASTER,
   20,
   _("+100 Cent"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_PITCH_P100},
  {INITIAL_MASTER,
   21,
   _("Prev temperament"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_TEMPERAMENT_PREV},
  {INITIAL_MASTER,
   22,
   _("Next temperament"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_TEMPERAMENT_NEXT},
  {INITIAL_MASTER,
   23,
   _("Transpose -"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_TRANSPOSE_DOWN},
  {INITIAL_MASTER,
   24,
   _("Transpose +"),
   GOSetter::P_BUTTON_DEFS + GOSetter::ID_SETTER_TRANSPOSE_UP},
  {INITIAL_METRONOME,
   25,
   _("On"),
   GOMetronome::P_BUTTON_DEFS + GOMetronome::ID_METRONOME_ON},
  {INITIAL_METRONOME,
   26,
   _("BPM +"),
   GOMetronome::P_BUTTON_DEFS + GOMetronome::ID_METRONOME_BEAT_P1},
  {INITIAL_METRONOME,
   27,
   _("BPM -"),
   GOMetronome::P_BUTTON_DEFS + GOMetronome::ID_METRONOME_BEAT_M1},
  {INITIAL_METRONOME,
   28,
   _("Measure -"),
   GOMetronome::P_BUTTON_DEFS + GOMetronome::ID_METRONOME_MEASURE_M1},
  {INITIAL_METRONOME,
   29,
   _("Measure +"),
   GOMetronome::P_BUTTON_DEFS + GOMetronome::ID_METRONOME_MEASURE_P1},
};

static const GOConfigEnum INITIAL_LOAD_TYPES({
  {wxT("N"), (int)GOInitialLoadType::LOAD_NONE},
  {wxT("Y"), (int)GOInitialLoadType::LOAD_LAST_USED},
  {wxT("First"), (int)GOInitialLoadType::LOAD_FIRST},
});

GOConfig::GOConfig(wxString instance)
  : m_InstanceName(instance),
    OrganSettingsPath(this, GENERAL, wxT("SettingPath"), wxEmptyString),
    OrganCachePath(this, GENERAL, wxT("CachePath"), wxEmptyString),
    Concurrency(this, GENERAL, wxT("Concurrency"), 0, MAX_CPU, 1),
    ReleaseConcurrency(this, GENERAL, wxT("ReleaseConcurrency"), 1, MAX_CPU, 1),
    LoadConcurrency(this, GENERAL, wxT("LoadConcurrency"), 0, MAX_CPU, 1),
    m_InterpolationType(
      this,
      GENERAL,
      wxT("InterpolationType"),
      INTERPOLATION_LINEAR,
      INTERPOLATION_POLYPHASE,
      INTERPOLATION_DEFAULT),
    WaveFormatBytesPerSample(this, GENERAL, wxT("WaveFormat"), 1, 4, 4),
    RecordDownmix(this, GENERAL, wxT("RecordDownmix"), false),
    AttackLoad(this, GENERAL, wxT("AttackLoad"), 0, 1, 1),
    LoopLoad(this, GENERAL, wxT("LoopLoad"), 0, 2, 2),
    ReleaseLoad(this, GENERAL, wxT("ReleaseLoad"), 0, 1, 1),
    ManageCache(this, GENERAL, wxT("ManageCache"), true),
    CompressCache(this, GENERAL, wxT("CompressCache"), false),
    LoadLastFile(
      this,
      GENERAL,
      wxT("LoadLastFile"),
      INITIAL_LOAD_TYPES,
      GOInitialLoadType::LOAD_LAST_USED),
    ODFCheck(this, GENERAL, wxT("StrictODFCheck"), false),
    ODFHw1Check(this, GENERAL, wxT("ODFHw1Check"), false),
    LoadChannels(this, GENERAL, wxT("Channels"), 0, 2, 2),
    LosslessCompression(this, GENERAL, wxT("LosslessCompression"), false),
    ManagePolyphony(this, GENERAL, wxT("ManagePolyphony"), true),
    ScaleRelease(this, GENERAL, wxT("ScaleRelease"), true),
    RandomizeSpeaking(this, GENERAL, wxT("RandomizeSpeaking"), true),
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
      GENERAL,
      wxT("MemoryLimit"),
      0,
      1024 * 1024,
      GOMemoryPool::GetSystemMemoryLimit()),
    SamplesPerBuffer(
      this,
      GENERAL,
      wxT("SamplesPerBuffer"),
      1,
      MAX_FRAME_SIZE,
      SAMPLES_PER_BUFFER_DEFAULT),
    SampleRate(
      this, GENERAL, wxT("SampleRate"), 1000, 192000, SAMPLE_RATE_DEFAULT),
    Volume(this, GENERAL, wxT("Volume"), -120, 20, -15),
    PolyphonyLimit(
      this, GENERAL, wxT("PolyphonyLimit"), 0, MAX_POLYPHONY, 2048),
    Preset(this, GENERAL, wxT("Preset"), 0, MAX_PRESET, 0),
    LanguageCode(this, GENERAL, wxT("Language"), wxEmptyString),
    BitsPerSample(this, GENERAL, wxT("BitsPerSample"), 8, 24, 24),
    Transpose(this, GENERAL, wxT("Transpose"), -11, 11, 0),
    MetronomeMeasure(this, wxT("Metronome"), wxT("Measure"), 0, 32, 4),
    MetronomeBPM(this, wxT("Metronome"), wxT("BPM"), 1, 500, 80),
    IsToAutoAddMidi(this, MIDI_IN, wxT("IsToAutoAddMidi"), true),
    IsToCheckMidiOnStart(this, MIDI_IN, wxT("IsToCheckMidiOnStart"), true),
    MidiRecorderOutputDevice(
      this, MIDI_OUT, wxT("MIDIRecorderDevice"), wxEmptyString),
    OrganPath(this, GENERAL, wxT("OrganPath"), wxEmptyString),
    OrganPackagePath(this, GENERAL, wxT("OrganPackagePath"), wxEmptyString),
    OrganCombinationsPath(this, GENERAL, wxT("CmbYamlPath"), wxEmptyString),
    ExportImportPath(this, GENERAL, wxT("CMBPath"), wxEmptyString),
    AudioRecorderPath(this, GENERAL, wxT("AudioRecorder"), wxEmptyString),
    MidiRecorderPath(this, GENERAL, wxT("MIDIRecorderPath"), wxEmptyString),
    MidiPlayerPath(this, GENERAL, wxT("MIDIPlayerPath"), wxEmptyString),
    CheckForUpdatesAtStartup(
      this, GENERAL, wxT("CheckForUpdatesAtStartup"), true),
    m_MidiIn(MIDI_IN),
    m_MidiOut(MIDI_OUT) {
  m_Temperaments.InitTemperaments();
}

GOOrgan *GOConfig::CloneOrgan(const GOOrgan &newOrgan) const {
  return new GORegisteredOrgan(newOrgan);
}

bool GOConfig::IsValidOrgan(const GOOrgan *pOrgan) const {
  bool isOk = dynamic_cast<const GORegisteredOrgan *>(pOrgan);

  if (!isOk) {
    wxLogMessage(
      wxT("The organ %s is not registered"), pOrgan->GetChurchName());
  }
  return isOk;
}

void GOConfig::LoadOrgans(GOConfigReader &cfg) {
  ClearOrgans();
  ClearArchives();

  unsigned organ_count = cfg.ReadInteger(
    CMBSetting, GENERAL, wxT("OrganCount"), 0, 99999, false, 0);
  unsigned archive_count = cfg.ReadInteger(
    CMBSetting, GENERAL, wxT("ArchiveCount"), 0, 99999, false, 0);

  for (unsigned i = 0; i < organ_count; i++)
    AddNewOrgan(new GORegisteredOrgan(
      *this, cfg, wxString::Format(wxT("Organ%03d"), i + 1)));
  for (unsigned i = 0; i < archive_count; i++)
    AddNewArchive(
      new GOArchiveFile(cfg, wxString::Format(wxT("Archive%03d"), i + 1)));
}

void GOConfig::SaveOrgans(GOConfigWriter &cfg) {
  const ptr_vector<GOOrgan> &organs = GetOrganList();
  const ptr_vector<GOArchiveFile> &archives = GetArchiveList();

  cfg.WriteInteger(GENERAL, wxT("ArchiveCount"), archives.size());
  for (unsigned i = 0; i < archives.size(); i++)
    archives[i]->Save(cfg, wxString::Format(wxT("Archive%03d"), i + 1));

  cfg.WriteInteger(GENERAL, wxT("OrganCount"), organs.size());
  for (unsigned i = 0; i < organs.size(); i++) {
    const auto pOo = organs[i];
    const GORegisteredOrgan *pO = dynamic_cast<const GORegisteredOrgan *>(pOo);

    if (pO)
      pO->Save(cfg, wxString::Format(wxT("Organ%03d"), i + 1), m_MidiMap);
  }
}

static void load_ports_config(
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

static void save_ports_config(
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

unsigned GOConfig::getMidiBuiltinCount() {
  return sizeof(INTERNAL_MIDI_DESCS) / sizeof(INTERNAL_MIDI_DESCS[0]);
}

static wxString get_old_style_initial_midi_group(unsigned index) {
  assert(index < GOConfig::getMidiBuiltinCount());
  const auto &desc = INTERNAL_MIDI_DESCS[index];

  return wxString::Format(
    INITIAL_MIDI_GROUP_DESCS[desc.m_group].m_SectionFmt, desc.m_index);
}

static wxString get_initial_midi_group(unsigned index) {
  return wxString::Format(WX_MIDI_INITIAL_FMT, index + 1);
}

void GOConfig::Load() {
  if (m_ConfigFileName.IsEmpty()) {
    LoadDefaults();
  }
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

    try {
      LoadOrgans(cfg);
    } catch (const wxString &error) {
      wxLogError(wxT("%s\n"), error);
    }

    m_MainWindowRect.x = cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowX"), -32000, 32000, false, 0);
    m_MainWindowRect.y = cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowY"), -32000, 32000, false, 0);
    m_MainWindowRect.width = (unsigned)cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowWidth"), 0, 32000, false, 0);
    m_MainWindowRect.height = (unsigned)cfg.ReadInteger(
      CMBSetting, wxT("UI"), wxT("MainWindowHeight"), 0, 32000, false, 0);

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
      CMBSetting,
      GOAudioDeviceConfig::WX_AUDIO_DEVICES,
      COUNT,
      0,
      200,
      false,
      0);

    if (count > 0)
      for (unsigned i = 0; i < count; i++) {
        m_AudioDeviceConfig.emplace_back();
        m_AudioDeviceConfig[i].Load(cfg, i + 1);
      }
    else // default device config
      m_AudioDeviceConfig.emplace_back(m_AudioGroups);

    load_ports_config(
      cfg, MIDI_PORTS, GOMidiPortFactory::getInstance(), m_MidiPortsConfig);

    int midiInitialCount = cfg.ReadInteger(
      CMBSetting, GENERAL, WX_MIDI_INITIAL_COUNT, 0, 9999, false, -1);

    if (midiInitialCount < 1) // old style config
      for (unsigned l = getMidiBuiltinCount(), i = 0; i < l; i++)
        m_InitialMidiObjects[i]->GetMidiReceiver()->Load(
          ODFCheck(), cfg, get_old_style_initial_midi_group(i), m_MidiMap);
    else
      for (unsigned l = midiInitialCount, i = 0; i < l; i++) {
        const wxString group = get_initial_midi_group(i);
        const wxString path = cfg.ReadString(CMBSetting, group, WX_PATH, false);
        const GOMidiObject::ObjectType objectType
          = (GOMidiObject::ObjectType)cfg.ReadEnum(
            CMBSetting, group, WX_OBJECT_TYPE, GOMidiObject::OBJECT_TYPES);
        GOConfigMidiObject *pObj = nullptr;

        if (path.IsEmpty()) {
          // An internal MIDI object. Match by receiverType, midiInputNumber
          const unsigned midiInputNumber
            = cfg.ReadInteger(CMBSetting, group, WX_MIDI_INPUT_NUMBER, 0, 999);

          pObj = FindMidiInitialObject(objectType, midiInputNumber);
          if (!pObj)
            wxLogWarning(
              _("Internal MIDI object %s:%u exists"),
              GOMidiObject::OBJECT_TYPES.GetName(objectType),
              midiInputNumber);
        } else { // Patch is present. Match the object by path
          auto it = m_InitialMidiObjectsByPath.find(path);

          if (it == m_InitialMidiObjectsByPath.end()) {
            const wxString name
              = cfg.ReadString(CMBSetting, group, WX_NAME, false);

            pObj = new GOConfigMidiObject(
              m_MidiMap, objectType, WX_USER_ADDED, path, name);
            m_InitialMidiObjects.push_back(pObj);
            m_InitialMidiObjectsByPath[path] = pObj;
          } else // A built-in initial MIDI object (button) matched by path
            pObj = it->second;
        }
        if (pObj)
          pObj->LoadMidiObject(cfg, group, m_MidiMap);
      }

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
  } catch (const wxString &error) {
    wxLogError(wxT("%s\n"), error);
  }
}

void GOConfig::LoadDefaults() {

  /* This has to be called in the wxApp context
     e.g.: after having done the wxApp::OnInit() call.

     As wx is waiting for the wxApp to have been initialized
     before calling th Get() method.
  */

  m_ConfigFileName = GOStdPath::GetConfigDir() + wxFileName::GetPathSeparator()
    + wxT("GrandOrgueConfig") + m_InstanceName;
  for (const auto &desc : INTERNAL_MIDI_DESCS) {
    const initial_midi_group_desc &groupDesc
      = INITIAL_MIDI_GROUP_DESCS[desc.m_group];
    const GOElementCreator::ButtonDefinitionEntry *pButtonDef
      = desc.p_ButtonDef;
    wxString path = pButtonDef ? GOMidiObjectContext::getPath(
                      pButtonDef->p_MidiContext, pButtonDef->name)
                               : WX_EMPTY;
    GOConfigMidiObject *pObj = new GOConfigMidiObject(
      m_MidiMap,
      groupDesc.m_ObjectType,
      groupDesc.m_GroupName,
      path.IsEmpty() ? wxString::Format(WX_FMT_D, desc.m_index) : path,
      desc.m_name);

    pObj->SetSenderType(groupDesc.m_SenderType);
    pObj->SetReceiverType(groupDesc.m_ReceiverType);
    pObj->SetShortcutReceiverType(groupDesc.m_ShortcutReceiverType);
    pObj->SetDivisionSenderType(groupDesc.m_DivisionalType);
    m_InitialMidiObjects.push_back(pObj);
    if (!path.IsEmpty())
      m_InitialMidiObjectsByPath[path] = pObj;
  }
  m_ResourceDir = GOStdPath::GetResourceDir();

  OrganPath.SetDefaultValue(GOStdPath::GetGrandOrgueSubDir(_("Organs")));
  OrganPackagePath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Organ packages")));
  OrganCachePath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(wxT("Cache") + m_InstanceName));
  OrganSettingsPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(wxT("Data") + m_InstanceName));
  OrganCombinationsPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Combinations")));
  ExportImportPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Settings")));
  AudioRecorderPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("Audio recordings")));
  MidiRecorderPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("MIDI recordings")));
  MidiPlayerPath.SetDefaultValue(
    GOStdPath::GetGrandOrgueSubDir(_("MIDI recordings")));
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

const wxString &GOConfig::GetEventGroup(unsigned index) const {
  assert(index < getMidiBuiltinCount());

  return INITIAL_MIDI_GROUP_DESCS[INTERNAL_MIDI_DESCS[index].m_group]
    .m_GroupName;
}

wxString GOConfig::GetEventTitle(unsigned index) {
  assert(index < getMidiBuiltinCount());
  return INTERNAL_MIDI_DESCS[index].m_name;
}

GOConfigMidiObject *GOConfig::GetMidiInitialObject(unsigned index) {
  assert(index < m_InitialMidiObjects.size());
  return m_InitialMidiObjects[index];
}

GOConfigMidiObject *GOConfig::FindMidiInitialObject(
  GOMidiObject::ObjectType objectType, unsigned midiInputNumber) {
  auto builInEnd = INTERNAL_MIDI_DESCS + getMidiBuiltinCount();
  auto foundDesc = std::find_if(
    INTERNAL_MIDI_DESCS, builInEnd, [=](const internal_midi_object_desc &desc) {
      return INITIAL_MIDI_GROUP_DESCS[desc.m_group].m_ObjectType == objectType
        && desc.m_index == midiInputNumber;
    });

  return foundDesc != builInEnd
    ? m_InitialMidiObjects[foundDesc - INTERNAL_MIDI_DESCS]
    : nullptr;
}

GOConfigMidiObject *GOConfig::FindMidiInitialObject(const wxString &path) {
  auto it = m_InitialMidiObjectsByPath.find(path);

  return it != m_InitialMidiObjectsByPath.end() ? it->second : nullptr;
}

const wxString GOConfig::GetPackageDirectory() {
  return m_ResourceDir + wxFileName::GetPathSeparator() + wxT("packages");
}

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

void GOConfig::SetAudioDeviceConfig(
  const std::vector<GOAudioDeviceConfig> &config) {
  if (!config.size())
    return;
  m_AudioDeviceConfig = config;
}

const unsigned GOConfig::GetTotalAudioChannels() const {
  unsigned channels = 0;

  for (const GOAudioDeviceConfig &deviceConfig : m_AudioDeviceConfig)
    channels += deviceConfig.GetChannels();
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
  SaveOrgans(cfg);

  cfg.WriteInteger(wxT("UI"), wxT("MainWindowX"), m_MainWindowRect.x);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowY"), m_MainWindowRect.y);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowWidth"), m_MainWindowRect.width);
  cfg.WriteInteger(wxT("UI"), wxT("MainWindowHeight"), m_MainWindowRect.height);

  m_Temperaments.Save(cfg);

  unsigned midiInitialBuiltinCount = getMidiBuiltinCount();
  unsigned midiInitialCount = m_InitialMidiObjects.size();

  cfg.WriteInteger(GENERAL, WX_MIDI_INITIAL_COUNT, midiInitialCount);
  for (unsigned i = 0; i < midiInitialCount; i++) {
    const wxString group = get_initial_midi_group(i);
    const GOConfigMidiObject *pObj = m_InitialMidiObjects[i];
    const unsigned midiInputNumber
      = i < midiInitialBuiltinCount ? INTERNAL_MIDI_DESCS[i].m_index : 0;

    cfg.WriteEnum(
      group, WX_OBJECT_TYPE, GOMidiObject::OBJECT_TYPES, pObj->GetObjectType());
    if (midiInputNumber) // matching by objectType, midiInputNumber
      cfg.WriteInteger(group, WX_MIDI_INPUT_NUMBER, midiInputNumber);
    else {
      // A user-added MIDI object or a built-in with a m_ButtonDef. Match it by
      // path
      cfg.WriteString(group, WX_PATH, pObj->GetMatchingBy());
      cfg.WriteString(group, WX_NAME, pObj->GetName());
    }
    pObj->SaveMidiObject(cfg, group, m_MidiMap);
  }

  for (unsigned i = 0; i < m_AudioGroups.size(); i++)
    cfg.WriteString(
      wxT("AudioGroups"),
      wxString::Format(wxT("Name%03d"), i + 1),
      m_AudioGroups[i]);
  cfg.WriteInteger(wxT("AudioGroups"), COUNT, m_AudioGroups.size());

  save_ports_config(
    cfg, SOUND_PORTS, GOSoundPortFactory::getInstance(), m_SoundPortsConfig);

  const unsigned audioDeviceCount = m_AudioDeviceConfig.size();

  for (unsigned i = 0; i < audioDeviceCount; i++)
    m_AudioDeviceConfig[i].Save(cfg, i + 1);
  cfg.WriteInteger(
    GOAudioDeviceConfig::WX_AUDIO_DEVICES, COUNT, audioDeviceCount);

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
  if (!go_rename_file(tmp_name, m_ConfigFileName))
    return;
}
