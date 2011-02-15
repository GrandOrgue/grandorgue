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

#ifndef MIDIEVENTS_H
#define MIDIEVENTS_H

extern const char* s_MIDIMessages[];
extern long i_MIDIMessages[];
extern const char* s_MIDIEvents0[];
extern const char* s_MIDIEvents1[];
extern const char* s_MIDIEvents2[];
extern const char* s_MIDIEvents3[];

extern const int i_MIDIEvents0[];
extern const int i_MIDIEvents1[];
extern const int i_MIDIEvents2[];

typedef struct
{
	const char** names;
	const int* events;
	const int count;
} MIDIEvents;

extern MIDIEvents g_MIDIEvents[6];

#endif
