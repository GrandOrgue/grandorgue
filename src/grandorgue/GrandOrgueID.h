/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#ifndef GRANDORGUEID_H
#define GRANDORGUEID_H

#include "GOrgueLimits.h"
#include <wx/defs.h>

enum
{
	ID_GOrgueFIRSTMENU = wxID_HIGHEST,

	ID_FILE_EXIT,
	ID_FILE_OPEN,
	ID_FILE_INSTALL,
	ID_FILE_LOAD,
	ID_LOAD_FAV_FIRST,
	ID_LOAD_FAV_LAST = ID_LOAD_FAV_FIRST + 9,
	ID_LOAD_LRU_FIRST,
	ID_LOAD_LRU_LAST = ID_LOAD_LRU_FIRST + 9,

	ID_FILE_RELOAD,
	ID_FILE_REVERT,
	ID_FILE_IMPORT_SETTINGS,
	ID_FILE_IMPORT_COMBINATIONS,
	ID_FILE_EXPORT,
	ID_FILE_CACHE,
	ID_FILE_CACHE_DELETE,
	ID_FILE_PROPERTIES,
	ID_FILE_SAVE,
	ID_FILE_CLOSE,

	ID_ORGAN_EDIT,
	ID_MIDI_LIST,

	ID_MIDI_RECORD,
	ID_MIDI_PLAY,
	ID_MIDI_MONITOR,
	ID_AUDIO_RECORD,
	ID_AUDIO_MEMSET,
	ID_AUDIO_PANIC,
	ID_AUDIO_STATE,
	ID_AUDIO_SETTINGS,

	ID_PRESET_0,
	ID_PRESET_LAST = ID_PRESET_0 + MAX_PRESET, 

	ID_TEMPERAMENT_0,
	ID_TEMPERAMENT_LAST = ID_TEMPERAMENT_0 + 1000,

	ID_VOLUME,
	ID_POLYPHONY,
	ID_MEMORY,
	ID_TRANSPOSE,
	ID_RELEASELENGTH,
	ID_RELEASELENGTH_SELECT,

	ID_METER_AUDIO_SPIN,
	ID_METER_AUDIO_LEFT,
	ID_METER_AUDIO_RIGHT,
	ID_METER_POLY_SPIN,
	ID_METER_POLY_GAUGE,
	ID_METER_FRAME_SPIN,
	ID_METER_TRANSPOSE_SPIN,
	ID_PANEL_MENU,
	ID_PANEL_FIRST,
	ID_PANEL_LAST = ID_PANEL_FIRST + 200,
};

#endif
