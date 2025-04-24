/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GORodgers.h"

uint8_t GORodgersChecksum(
  const std::vector<uint8_t> &msg, unsigned start, unsigned len) {
  uint8_t sum = 0;
  for (unsigned i = 0; i < len; i++)
    sum += msg[start + i];
  return (-sum) & 0x7f;
}

MIDI_BIT_STATE
GORodgersGetBit(
  unsigned stop, unsigned offset, const std::vector<uint8_t> data) {
  unsigned start = offset * 7;
  if (stop < start)
    return MIDI_BIT_STATE::MIDI_BIT_NOT_PRESENT;
  stop -= start;
  unsigned pos = stop / 7;
  unsigned bit = stop - pos * 7;
  if (pos >= data.size())
    return MIDI_BIT_STATE::MIDI_BIT_NOT_PRESENT;
  if (data[pos] & (1 << bit))
    return MIDI_BIT_STATE::MIDI_BIT_SET;
  else
    return MIDI_BIT_STATE::MIDI_BIT_CLEAR;
}
unsigned GORodgersSetBit(
  unsigned stop, bool state, std::vector<uint8_t> &data) {
  unsigned offset = stop / 7;
  unsigned bit = stop - offset * 7;
  if (data.size() <= offset)
    data.resize(offset + 1);
  uint8_t &byte = data[offset];
  if (state)
    byte |= 1 << bit;
  else
    byte &= ~(1 << bit);

  return offset;
}
