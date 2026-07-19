/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestMidiSendProxy.h"

#include "midi/elements/GOMidiSendProxy.h"

const std::string GOTestMidiSendProxy::TEST_NAME = "GOTestMidiSendProxy";

void GOTestMidiSendProxy::TestDefaultState() {
  GOMidiSendProxy proxy;

  GOAssert(proxy.IsToSendMidi(), "IsToSendMidi() should be true by default");
  GOAssert(
    proxy.IsToRecordMidi(), "IsToRecordMidi() should be true by default");
}

void GOTestMidiSendProxy::TestToSendMidiIsIndependent() {
  GOMidiSendProxy proxy;

  proxy.SetToSendMidi(false);
  GOAssert(
    !proxy.IsToSendMidi(),
    "IsToSendMidi() should be false after SetToSendMidi(false)");
  GOAssert(
    proxy.IsToRecordMidi(),
    "IsToRecordMidi() should stay true after SetToSendMidi(false)");

  proxy.SetToSendMidi(true);
  GOAssert(
    proxy.IsToSendMidi(),
    "IsToSendMidi() should be true after SetToSendMidi(true)");
}

void GOTestMidiSendProxy::TestToRecordMidiIsIndependent() {
  GOMidiSendProxy proxy;

  proxy.SetToRecordMidi(false);
  GOAssert(
    !proxy.IsToRecordMidi(),
    "IsToRecordMidi() should be false after SetToRecordMidi(false)");
  GOAssert(
    proxy.IsToSendMidi(),
    "IsToSendMidi() should stay true after SetToRecordMidi(false)");

  proxy.SetToRecordMidi(true);
  GOAssert(
    proxy.IsToRecordMidi(),
    "IsToRecordMidi() should be true after SetToRecordMidi(true)");
}

void GOTestMidiSendProxy::TestStateRestorer() {
  GOMidiSendProxy proxy;

  proxy.SetToSendMidi(false);
  proxy.SetToRecordMidi(true);

  {
    GOMidiSendProxy::StateRestorer restorer(proxy);

    proxy.SetToSendMidi(true);
    proxy.SetToRecordMidi(false);
  }

  GOAssert(
    !proxy.IsToSendMidi(),
    "StateRestorer should restore IsToSendMidi() to its saved value");
  GOAssert(
    proxy.IsToRecordMidi(),
    "StateRestorer should restore IsToRecordMidi() to its saved value");
}

void GOTestMidiSendProxy::run() {
  TestDefaultState();
  TestToSendMidiIsIndependent();
  TestToRecordMidiIsIndependent();
  TestStateRestorer();
}
