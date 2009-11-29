/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#pragma once

const char* s_MIDIMessages[] =
{
	"Previous Program",
	"Next Program",
	"Enclosure 1",
	"Enclosure 2",
	"Enclosure 3",
	"Enclosure 4",
	"Enclosure 5",
	"Enclosure 6",
	"Manual 1 (Pedal)",
	"Manual 2 (Great)",
	"Manual 3 (Swell)",
	"Manual 4 (Choir)",
	"Manual 5 (Solo)",
	"Manual 6 (Echo)",
	"Stop Changes",
	"Memory Set",
};

long i_MIDIMessages[] =
{
	0xC400,
	0xC401,
    0xB001,
	0xB101,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x9100,
	0x9000,
	0x9200,
	0x9300,
	0x9500,
	0x9600,
	0x9400,
	0x0000,
};

const char* s_MIDIEvents0[] =
	{
	"(none)",
	"Bx Controller",
	};

const char* s_MIDIEvents1[] =
	{
	"(none)",
	"9x Note On",
	};

const char* s_MIDIEvents2[] =
	{
	"(none)",
	"9x Note On",
	"Bx Controller",
	"Cx Program Change",
	};

const char* s_MIDIEvents3[] =
	{
	"(none)",
	"Cx Program Change",
	};

const int i_MIDIEvents0[] = {0x0000,0xB000};
const int i_MIDIEvents1[] = {0x0000,0x9000};
const int i_MIDIEvents2[] = {0x0000,0x9000,0xB000,0xC000};

struct
{
	const char** names;
	const int* events;
	const int count;
} g_MIDIEvents[6] =
{
	{s_MIDIEvents0, i_MIDIEvents0, 2},
	{s_MIDIEvents1, i_MIDIEvents1, 2},
	{s_MIDIEvents2, i_MIDIEvents2, 4},
	{s_MIDIEvents2, i_MIDIEvents2, 4},
	{s_MIDIEvents2, i_MIDIEvents2, 4},
	{s_MIDIEvents2, i_MIDIEvents2, 4},
};
