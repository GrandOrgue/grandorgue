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

#include "midi/GOMidiSystem.h"
#include "midi/ports/GOMidiInPort.h"
#include "midi/ports/GOMidiOutPort.h"

class GOTestMidiInput : public GOMidiInPort {
public:
  unsigned m_Opens = 0;
  bool m_Available = true;

  GOTestMidiInput(GOMidiSystem &midi, const wxString &name)
    : GOMidiInPort(&midi, "Test", "test", name, name) {}

  bool Open(unsigned id, int channelShift) override {
    Close();
    ++m_Opens;
    m_IsActive = m_Available;
    return GOMidiInPort::Open(id, channelShift);
  }

  int GetChannelShift() const { return m_ChannelShift; }
};

class GOTestMidiOutput : public GOMidiOutPort {
public:
  unsigned m_Opens = 0;
  bool m_Available = true;

  GOTestMidiOutput(GOMidiSystem &midi, const wxString &name)
    : GOMidiOutPort(&midi, "Test", "test", name, name) {}

  bool Open(unsigned id) override {
    Close();
    ++m_Opens;
    m_IsActive = m_Available;
    return GOMidiOutPort::Open(id);
  }
};

static void ConfigureTestPorts(GOConfig &config) {
  GOPortsConfig ports;

  ports.SetConfigEnabled("Rt", false);
  config.SetMidiPortsConfig(ports);
}

void GOTestMidiSystem::TestLateDiscovery() {
  GOConfig config(GetName(), "");
  ConfigureTestPorts(config);
  GOMidiSystem midi(config);
  GOMidiDeviceConfig *input = config.m_MidiIn.Append(
    "Keyboard", "^Input:[0-9]+$", "Test", "test", true);
  GOMidiDeviceConfig *output = config.m_MidiOut.Append(
    "Console", "^Output:[0-9]+$", "Test", "test", true);

  input->m_ChannelShift = 2;
  midi.Open();
  GOAssert(
    midi.m_DiscoveryTimer.IsRunning() && midi.m_DiscoveryTimer.IsOneShot(),
    "Absent configured ports must start discovery");

  GOTestMidiInput *in = new GOTestMidiInput(midi, "Input:28");
  GOTestMidiOutput *out = new GOTestMidiOutput(midi, "Output:28");

  midi.m_midi_in_devices.push_back(in);
  midi.m_midi_out_devices.push_back(out);
  wxTimerEvent event;

  midi.OnDiscoveryTimer(event);
  GOAssert(in->IsActive(), "A late input must open without a restart");
  GOAssert(out->IsActive(), "A late output must open without a restart");
  GOAssert(
    in->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName("Keyboard"),
    "A late input must retain its logical ID");
  GOAssert(
    in->GetChannelShift() == 2, "A late input must retain its channel shift");
  GOAssert(
    out->GetID() == midi.GetMidiMap().GetDeviceIdByLogicalName("Console"),
    "A late output must retain its logical ID");
  GOAssert(!midi.m_DiscoveryTimer.IsRunning(), "Discovery must stop after all configured ports connect");
}

void GOTestMidiSystem::TestFailedOpen() {
  GOConfig config(GetName(), "");
  ConfigureTestPorts(config);
  GOMidiSystem midi(config);

  config.m_MidiIn.Append("Keyboard", "^Input:[0-9]+$", "Test", "test", true);
  config.m_MidiOut.Append("Console", "^Output:[0-9]+$", "Test", "test", true);
  GOTestMidiInput *unavailableIn = new GOTestMidiInput(midi, "Input:28");
  GOTestMidiOutput *unavailableOut = new GOTestMidiOutput(midi, "Output:28");

  unavailableIn->m_Available = false;
  unavailableOut->m_Available = false;
  midi.m_midi_in_devices.push_back(unavailableIn);
  midi.m_midi_out_devices.push_back(unavailableOut);
  midi.Open();
  GOAssert(
    midi.m_DiscoveryTimer.IsRunning(),
    "A failed open must continue discovery");

  GOTestMidiInput *readyIn = new GOTestMidiInput(midi, "Input:32");
  GOTestMidiOutput *readyOut = new GOTestMidiOutput(midi, "Output:32");

  midi.m_midi_in_devices.push_back(readyIn);
  midi.m_midi_out_devices.push_back(readyOut);
  wxTimerEvent event;

  midi.OnDiscoveryTimer(event);
  GOAssert(readyIn->IsActive(), "Discovery must retry an input at a new address");
  GOAssert(readyOut->IsActive(), "Discovery must retry an output at a new address");
  GOAssert(!midi.m_DiscoveryTimer.IsRunning(), "Discovery must stop after successful retries");
}

void GOTestMidiSystem::run() {
  wxAppConsole app;
  std::unique_ptr<wxEventLoopBase> loop(app.GetTraits()->CreateEventLoop());
  wxEventLoopActivator activeLoop(loop.get());

  TestLateDiscovery();
  TestFailedOpen();
}
