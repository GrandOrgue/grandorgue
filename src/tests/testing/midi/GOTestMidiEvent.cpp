/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestMidiEvent.h"

#include <format>
#include <vector>

#include "midi/GOMidiMap.h"
#include "midi/events/GOMidiEvent.h"
#include "midi/events/GORodgers.h"

#include "GOTestScope.h"

const std::string GOTestMidiEvent::TEST_NAME = "GOTestMidiEvent";

/**
 * Builds a Rodgers stop change SysEx: F0 41 dd 30 12 01 kk <data> cs F7
 */
static std::vector<unsigned char> make_rodgers_stop_change(
  unsigned char device,
  unsigned char key,
  const std::vector<unsigned char> &data) {
  std::vector<unsigned char> msg = {0xF0, 0x41, device, 0x30, 0x12, 0x01, key};

  msg.insert(msg.end(), data.begin(), data.end());
  msg.push_back(0);
  msg.push_back(0xF7);
  msg[msg.size() - 2] = GORodgersChecksum(msg, 5, msg.size() - 7);
  return msg;
}

void GOTestMidiEvent::TestRodgersStopChange() {
  GOMidiMap map;
  GOMidiEvent event;
  const std::vector<unsigned char> data = {0x12, 0x34, 0x56};

  event.FromMidi(make_rodgers_stop_change(3, 7, data), map);

  GOAssert(
    event.GetMidiType() == GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE,
    "A valid Rodgers stop change message should be recognized");
  GOAssert(
    event.GetChannel() == 3,
    std::format("Device should be 3, got {}", event.GetChannel()));
  GOAssert(
    event.GetKey() == 7,
    std::format("Key should be 7, got {}", event.GetKey()));
  GOAssert(
    event.GetData() == data, "Data should be equal to the message payload");
}

void GOTestMidiEvent::TestRodgersStopChangeEmptyData() {
  GOMidiMap map;
  GOMidiEvent event;

  /* the shortest valid message: 9 bytes */
  event.FromMidi(make_rodgers_stop_change(1, 2, {}), map);

  GOAssert(
    event.GetMidiType() == GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE,
    "A Rodgers stop change message without data should be recognized");
  GOAssert(
    event.GetData().empty(),
    std::format("Data should be empty, got {} bytes", event.GetData().size()));
}

void GOTestMidiEvent::TestRodgersStopChangeTooShort() {
  GOMidiMap map;
  GOMidiEvent event;

  /* F0 41 dd 30 12 01 cs F7: 8 bytes with a valid checksum and no key byte */
  std::vector<unsigned char> msg
    = {0xF0, 0x41, 0x01, 0x30, 0x12, 0x01, 0, 0xF7};

  msg[6] = GORodgersChecksum(msg, 5, 1);
  event.FromMidi(msg, map);

  GOAssert(
    event.GetMidiType() == GOMidiEvent::MIDI_NONE,
    "An 8-byte Rodgers stop change message should be ignored");
}

void GOTestMidiEvent::run() {
  GO_RUN_TEST(TestRodgersStopChange())
  GO_RUN_TEST(TestRodgersStopChangeEmptyData())
  GO_RUN_TEST(TestRodgersStopChangeTooShort())
}
