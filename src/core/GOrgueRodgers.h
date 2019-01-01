/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GORGUERODERS_H
#define GORGUERODERS_H

#include <vector>
#include <stdint.h>

/* Bit test of the data field; returns clear, set or bit not in data */
enum class MIDI_BIT_STATE {
	MIDI_BIT_CLEAR,
	MIDI_BIT_SET,
	MIDI_BIT_NOT_PRESENT
};


uint8_t GORodgersChecksum(const std::vector<uint8_t>& msg, unsigned start, unsigned len);

MIDI_BIT_STATE GORodgersGetBit(unsigned stop, unsigned offset, const std::vector<uint8_t> data);
unsigned GORodgersSetBit(unsigned stop, bool state, std::vector<uint8_t>& data);

#endif
