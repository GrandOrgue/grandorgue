/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestWebRemoteDefaults.h"

#include <filesystem>
#include <format>
#include <fstream>

#include <wx/init.h>

#include "GOTestScope.h"
#include "config/GOConfigMidiObject.h"
#include "midi/elements/GOMidiReceiver.h"
#include "midi/ports/GOMidiWebInPort.h"

const std::string GOTestWebRemoteDefaults::TEST_NAME
  = "GOTestWebRemoteDefaults";

// a file that does not exist, so Load() only applies the defaults
static const std::string NO_CONFIG_FILE = "/nonexistent/GrandOrgueConfigTest";

// GOConfig::Load() goes through wxStandardPaths, which needs a wxApp around
bool GOTestWebRemoteDefaults::setUp() {
  return GOTest::setUp() && wxInitialize();
}

bool GOTestWebRemoteDefaults::tearDown() {
  wxUninitialize();
  return GOTest::tearDown();
}

// keep in sync with the defaults in resource/web-remote.html
static const struct {
  const char *path;
  int note;
  GOMidiReceiverMessageType type;
} EXPECTED[] = {
  {"Setter/Set", 100, MIDI_M_NOTE_ON}, // the only toggle, flips on each tap
  {"Setter/GC", 101, MIDI_M_NOTE},
  {"Sequencer/L0", 102, MIDI_M_NOTE},
  {"Sequencer/L5", 107, MIDI_M_NOTE},
  {"Sequencer/L9", 111, MIDI_M_NOTE},
  {"Sequencer/Prev", 112, MIDI_M_NOTE},
  {"Sequencer/Next", 113, MIDI_M_NOTE},
  {"Volume/VolumeDown", 114, MIDI_M_NOTE},
  {"Volume/VolumeUp", 115, MIDI_M_NOTE},
};

void GOTestWebRemoteDefaults::TestSetterButtonsAreMapped() {
  GOConfig config(TEST_NAME, NO_CONFIG_FILE);

  config.Load();

  const unsigned webId = config.GetMidiMap().GetDeviceIdByLogicalName(
    GOMidiWebInPort::DEVICE_NAME);

  GOAssert(webId != 0, "Web Remote should be registered in the midi map");

  for (const auto &x : EXPECTED) {
    const GOConfigMidiObject *pObj
      = config.FindMidiInitialObject(wxString(x.path));

    GOAssert(pObj != nullptr, std::format("{} should exist", x.path));

    const GOMidiReceiver &recv = *pObj->GetMidiReceiver();

    GOAssert(
      recv.GetEventCount() == 1,
      std::format(
        "{} should have exactly one event, has {}",
        x.path,
        recv.GetEventCount()));

    const GOMidiReceiverEventPattern &e = recv.GetEvent(0);

    GOAssert(
      e.type == x.type,
      std::format(
        "{} should have type {}, has {}", x.path, (int)x.type, (int)e.type));
    GOAssert(
      e.deviceId == webId,
      std::format("{} should listen to the Web Remote device", x.path));
    GOAssert(
      e.channel == 16 && e.key == x.note,
      std::format(
        "{} should be note {} on channel 16, got note {} channel {}",
        x.path,
        x.note,
        e.key,
        (int)e.channel));
  }
}

void GOTestWebRemoteDefaults::TestOtherButtonsAreNot() {
  GOConfig config(TEST_NAME, NO_CONFIG_FILE);

  config.Load();

  for (const char *path :
       {"Sequencer/Current", "Sequencer/M10", "Sequencer/P10"}) {
    const GOConfigMidiObject *pObj
      = config.FindMidiInitialObject(wxString(path));

    GOAssert(pObj != nullptr, std::format("{} should exist", path));
    GOAssert(
      pObj->GetMidiReceiver()->GetEventCount() == 0,
      std::format("{} should not be mapped by default", path));
  }
}

// A console piston already mapped to "<" must keep working: the Web Remote
// event is added next to it, not instead of it.
void GOTestWebRemoteDefaults::TestKeepsMappingsFromOtherDevices() {
  const std::filesystem::path cfgPath
    = std::filesystem::temp_directory_path() / "GOTestWebRemoteDefaults.cfg";

  {
    std::ofstream out(cfgPath);

    out << "\xEF\xBB\xBF[General]\nMidiInitialCount=1\n"
           "[MidiInitial001]\nObjectType=Button\nPath=Sequencer/Prev\n"
           "Name=Previous Memory\nReceiverType=Setter\nSenderType=Button\n"
           "ShortcutReceiverType=Button\nNumberOfMIDIEvents=1\n"
           "MIDIDevice001=alsa: CASIO USB-MIDI MIDI 1\nMIDIEventType001=Note\n"
           "MIDIChannel001=1\nMIDIKey001=95\nMIDILowerLimit001=0\n"
           "MIDIUpperLimit001=1\n";
  }

  GOConfig config(TEST_NAME, cfgPath.string());

  config.Load();
  std::filesystem::remove(cfgPath);

  const unsigned webId = config.GetMidiMap().GetDeviceIdByLogicalName(
    GOMidiWebInPort::DEVICE_NAME);
  const GOMidiReceiver &recv
    = *config.FindMidiInitialObject(wxString("Sequencer/Prev"))
         ->GetMidiReceiver();

  GOAssert(
    recv.GetEventCount() == 2,
    std::format("Prev should have both events, has {}", recv.GetEventCount()));
  GOAssert(
    recv.GetEvent(0).key == 95 && recv.GetEvent(0).deviceId != webId,
    "the console mapping should be kept as the first event");
  GOAssert(
    recv.GetEvent(1).key == 112 && recv.GetEvent(1).deviceId == webId,
    "the Web Remote event should be added after it");
}

void GOTestWebRemoteDefaults::run() {
  GO_RUN_TEST(TestSetterButtonsAreMapped())
  GO_RUN_TEST(TestOtherButtonsAreNot())
  GO_RUN_TEST(TestKeepsMappingsFromOtherDevices())
}
