/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestMidiSystem.h"

#include <memory>

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/evtloop.h>
#include <wx/timer.h>

#include "midi/GOMidiSystem.h"
#include "midi/ports/GOMidiInPort.h"
#include "midi/ports/GOMidiOutPort.h"

// Uses simulated ports and a headless wxWidgets event loop, so it needs
// no MIDI hardware. It verifies that configured input and output ports
// which appear after startup open with their saved mappings, that failed
// opens are retried if their address changes, and that discovery stops
// after success.

namespace {

constexpr int CHANNEL_SHIFT = 2;
constexpr const char *KEYBOARD = "Keyboard";
constexpr const char *CONSOLE = "Console";
constexpr const char *KEYBOARD_REGEX = "^Input:[0-9]+$";
constexpr const char *CONSOLE_REGEX = "^Output:[0-9]+$";
constexpr const char *PORT_NAME = "Test";
constexpr const char *API_NAME = "test";
constexpr const char *INPUT_28 = "Input:28";
constexpr const char *OUTPUT_28 = "Output:28";
constexpr const char *INPUT_32 = "Input:32";
constexpr const char *OUTPUT_32 = "Output:32";

// Shared state of the simulated input and output ports. The base classes
// declare different Open() signatures (the input takes a channel shift),
// so each concrete class keeps its own override.
template <class Base>
class TestMidiPort : public Base {
protected:
  const bool m_Available;

public:
  unsigned m_Opens = 0;

  TestMidiPort(
    GOMidiSystem &midi,
    const wxString &name,
    bool available = true)
    : Base(&midi, PORT_NAME, API_NAME, name, name),
      m_Available(available) {}

  bool BeginOpen() {
    ++m_Opens;
    return m_Available;
  }
};

class TestMidiInput : public TestMidiPort<GOMidiInPort> {
public:
  using TestMidiPort<GOMidiInPort>::TestMidiPort;

  bool Open(unsigned id, int channelShift) override {
    m_IsActive = m_Available;
    return BeginOpen() && GOMidiInPort::Open(id, channelShift);
  }

  int GetChannelShift() const { return m_ChannelShift; }
};

class TestMidiOutput : public TestMidiPort<GOMidiOutPort> {
public:
  using TestMidiPort<GOMidiOutPort>::TestMidiPort;

  bool Open(unsigned id) override {
    m_IsActive = m_Available;
    return BeginOpen() && GOMidiOutPort::Open(id);
  }

  void SendData(std::vector<unsigned char> &msg) override {}
};

void configure_test_ports(GOConfig &config) {
  GOPortsConfig ports;

  ports.SetConfigEnabled("Rt", false);
  config.SetMidiPortsConfig(ports);
}

}  // namespace

void GOTestMidiSystem::TestLateDiscovery() {
  GOConfig config(GetName(), "");
  configure_test_ports(config);
  GOMidiSystem midi(config);
  GOMidiDeviceConfig *input = config.m_MidiIn.Append(
    KEYBOARD, KEYBOARD_REGEX, PORT_NAME, API_NAME, true);
  config.m_MidiOut.Append(CONSOLE, CONSOLE_REGEX, PORT_NAME, API_NAME, true);

  input->m_ChannelShift = CHANNEL_SHIFT;
  midi.Open();
  GOAssert(
    midi.HasPendingDevices(),
    "Configured ports must stay pending while their devices are absent");
  GOAssert(
    midi.m_pDiscoveryTimer->IsRunning(),
    "Absent configured ports must start discovery");

  TestMidiInput *in = new TestMidiInput(midi, INPUT_28);
  TestMidiOutput *out = new TestMidiOutput(midi, OUTPUT_28);

  midi.m_midi_in_devices.push_back(in);
  midi.m_midi_out_devices.push_back(out);
  wxTimerEvent event;

  midi.OnDiscoveryTimer(event);
  GOAssert(in->IsActive(), "A late input must open without a restart");
  GOAssert(out->IsActive(), "A late output must open without a restart");
  GOAssert(
    in->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName(KEYBOARD),
    "A late input must retain its logical ID");
  GOAssert(
    in->GetChannelShift() == CHANNEL_SHIFT,
    "A late input must retain its channel shift");
  GOAssert(
    out->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName(CONSOLE),
    "A late output must retain its logical ID");
  GOAssert(
    !midi.HasPendingDevices(),
    "Discovery must stop after all configured ports connect");
  GOAssert(
    !midi.m_pDiscoveryTimer->IsRunning(),
    "Discovery must stop after all configured ports connect");
}

void GOTestMidiSystem::TestFailedOpen() {
  GOConfig config(GetName(), "");
  configure_test_ports(config);
  GOMidiSystem midi(config);

  config.m_MidiIn.Append(KEYBOARD, KEYBOARD_REGEX, PORT_NAME, API_NAME, true);
  config.m_MidiOut.Append(CONSOLE, CONSOLE_REGEX, PORT_NAME, API_NAME, true);
  TestMidiInput *unavailableIn = new TestMidiInput(midi, INPUT_28, false);
  TestMidiOutput *unavailableOut = new TestMidiOutput(midi, OUTPUT_28, false);

  midi.m_midi_in_devices.push_back(unavailableIn);
  midi.m_midi_out_devices.push_back(unavailableOut);
  midi.Open();
  GOAssert(
    midi.HasPendingDevices(),
    "Failed opens must keep the configuration pending");
  GOAssert(
    midi.m_pDiscoveryTimer->IsRunning(),
    "A failed open must continue discovery");

  TestMidiInput *readyIn = new TestMidiInput(midi, INPUT_32);
  TestMidiOutput *readyOut = new TestMidiOutput(midi, OUTPUT_32);

  midi.m_midi_in_devices.push_back(readyIn);
  midi.m_midi_out_devices.push_back(readyOut);
  wxTimerEvent event;

  midi.OnDiscoveryTimer(event);
  GOAssert(
    readyIn->IsActive(),
    "Discovery must retry an input at a new address");
  GOAssert(
    readyOut->IsActive(),
    "Discovery must retry an output at a new address");
  GOAssert(
    readyIn->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName(KEYBOARD),
    "A retried input must keep its saved logical mapping");
  GOAssert(
    readyOut->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName(CONSOLE),
    "A retried output must keep its saved logical mapping");
  GOAssert(
    !midi.HasPendingDevices(),
    "Discovery must stop after successful retries");
  GOAssert(
    !midi.m_pDiscoveryTimer->IsRunning(),
    "Discovery must stop after successful retries");
}

void GOTestMidiSystem::run() {
  wxAppConsole app;
  std::unique_ptr<wxEventLoopBase> loop(app.GetTraits()->CreateEventLoop());
  wxEventLoopActivator activeLoop(loop.get());

  TestLateDiscovery();
  TestFailedOpen();
}
