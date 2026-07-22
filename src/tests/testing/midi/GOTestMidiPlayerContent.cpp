/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestMidiPlayerContent.h"

#include <format>
#include <vector>

#include "midi/GOMidiPlayerContent.h"
#include "midi/events/GOMidiEvent.h"

const std::string GOTestMidiPlayerContent::TEST_NAME
  = "GOTestMidiPlayerContent";

void GOTestMidiPlayerContent::TestIsMidiInputNumberMappingUsable() {
  GOAssert(
    GOMidiPlayerContent::isMidiInputNumberMappingUsable({1, 2, 3, 4}),
    "Distinct values in [1, 16] should be usable");
  GOAssert(
    GOMidiPlayerContent::isMidiInputNumberMappingUsable({}),
    "An empty list should be usable");
  GOAssert(
    !GOMidiPlayerContent::isMidiInputNumberMappingUsable({-1, 2, 3}),
    "A missing (-1) MIDIInputNumber should not be usable");
  GOAssert(
    !GOMidiPlayerContent::isMidiInputNumberMappingUsable({0, 2, 3}),
    "MIDIInputNumber 0 should not be usable (out of [1, 16] range)");
  GOAssert(
    !GOMidiPlayerContent::isMidiInputNumberMappingUsable({1, 17, 3}),
    "MIDIInputNumber 17 should not be usable (out of [1, 16] range)");
  GOAssert(
    !GOMidiPlayerContent::isMidiInputNumberMappingUsable({1, 2, 2, 4}),
    "Duplicate MIDIInputNumber values should not be usable");
}

void GOTestMidiPlayerContent::TestComputeManualChannelsPedalFirst() {
  const std::vector<unsigned> channelsWithPedal
    = GOMidiPlayerContent::computeManualChannels(
      true, {-1, -1, -1, -1}, GOConfig::MIDI_PLAY_CHANNELS_PEDAL_FIRST);

  GOAssert(
    channelsWithPedal == std::vector<unsigned>({1, 2, 3, 4}),
    std::format(
      "PEDAL_FIRST with a pedal should put the pedal on channel 1, got "
      "{} entries, first: {}",
      channelsWithPedal.size(),
      channelsWithPedal.empty() ? 0 : channelsWithPedal[0]));

  const std::vector<unsigned> channelsWithoutPedal
    = GOMidiPlayerContent::computeManualChannels(
      false, {-1, -1, -1}, GOConfig::MIDI_PLAY_CHANNELS_PEDAL_FIRST);

  GOAssert(
    channelsWithoutPedal == std::vector<unsigned>({1, 2, 3}),
    "PEDAL_FIRST without a pedal should still start manuals at channel 1");
}

void GOTestMidiPlayerContent::TestComputeManualChannelsPedalLast() {
  const std::vector<unsigned> channelsWithPedal
    = GOMidiPlayerContent::computeManualChannels(
      true, {-1, -1, -1, -1}, GOConfig::MIDI_PLAY_CHANNELS_PEDAL_LAST);

  GOAssert(
    channelsWithPedal == std::vector<unsigned>({4, 1, 2, 3}),
    "PEDAL_LAST with a pedal should put the pedal on the last channel");

  const std::vector<unsigned> channelsWithoutPedal
    = GOMidiPlayerContent::computeManualChannels(
      false, {-1, -1, -1}, GOConfig::MIDI_PLAY_CHANNELS_PEDAL_LAST);

  GOAssert(
    channelsWithoutPedal == std::vector<unsigned>({1, 2, 3}),
    "PEDAL_LAST without a pedal should assign manuals channels 1..N");
}

void GOTestMidiPlayerContent::TestComputeManualChannelsUseInputNumber() {
  const std::vector<unsigned> channels
    = GOMidiPlayerContent::computeManualChannels(
      true, {1, 2, 3, 4}, GOConfig::MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER);

  GOAssert(
    channels == std::vector<unsigned>({1, 2, 3, 4}),
    "USE_INPUT_NUMBER with a usable mapping should use it as-is");
}

void GOTestMidiPlayerContent::
  TestComputeManualChannelsUseInputNumberFallback() {
  const std::vector<unsigned> channels
    = GOMidiPlayerContent::computeManualChannels(
      true, {-1, -1, -1, -1}, GOConfig::MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER);

  GOAssert(
    channels == std::vector<unsigned>({4, 1, 2, 3}),
    "USE_INPUT_NUMBER with an unusable mapping should fall back to "
    "PEDAL_LAST");
}

void GOTestMidiPlayerContent::TestHasNativeHeader() {
  std::vector<GOMidiEvent> emptyEvents;

  GOAssert(
    !GOMidiPlayerContent::hasNativeHeader(emptyEvents),
    "An empty event list should not have a native header");

  GOMidiEvent clearEvent;
  clearEvent.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_CLEAR);

  std::vector<GOMidiEvent> nativeEvents({clearEvent});

  GOAssert(
    GOMidiPlayerContent::hasNativeHeader(nativeEvents),
    "Events starting with MIDI_SYSEX_GO_CLEAR should have a native header");

  GOMidiEvent noteEvent;
  noteEvent.SetMidiType(GOMidiEvent::MIDI_NOTE);

  std::vector<GOMidiEvent> foreignEvents({noteEvent});

  GOAssert(
    !GOMidiPlayerContent::hasNativeHeader(foreignEvents),
    "Events not starting with MIDI_SYSEX_GO_CLEAR should not have a native "
    "header");
}

void GOTestMidiPlayerContent::run() {
  TestIsMidiInputNumberMappingUsable();
  TestComputeManualChannelsPedalFirst();
  TestComputeManualChannelsPedalLast();
  TestComputeManualChannelsUseInputNumber();
  TestComputeManualChannelsUseInputNumberFallback();
  TestHasNativeHeader();
}
