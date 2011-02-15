/*
 * GrandOrgue - a free pipe organ simulator based on MyOrgan
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

#ifndef GRANDORGUEID_H
#define GRANDORGUEID_H

#include <wx/wx.h>

enum
{
	ID_GOrgueFIRSTMENU = wxID_HIGHEST,

	ID_FILE_OPEN,
	ID_FILE_RELOAD,
	ID_FILE_REVERT,
	ID_FILE_LOAD,
	ID_FILE_SAVE,
	ID_FILE_CACHE,
	ID_FILE_PROPERTIES,

	ID_AUDIO_RECORD,
	ID_AUDIO_MEMSET,
	ID_AUDIO_PANIC,
    ID_AUDIO_SETTINGS,

    // Added by Graham Goode Nov 2009
    ID_VOLUME,
    ID_POLYPHONY,
    ID_MEMORY,
    ID_TRANSPOSE,
    //

	ID_METER_AUDIO_SPIN,
	ID_METER_AUDIO_LEFT,
	ID_METER_AUDIO_RIGHT,
	ID_METER_POLY_SPIN,
	ID_METER_POLY_GAUGE,
	ID_METER_FRAME_SPIN,
	ID_METER_TRANSPOSE_SPIN,
};

#endif
