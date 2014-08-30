/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueRtHelpers.h"

#include <wx/intl.h>

const wxChar* GOrgueRtHelpers::GetApiName(const RtAudio::Api api)
{
	switch (api)
	{
	case RtAudio::LINUX_ALSA:
		return _("Alsa");
	case RtAudio::LINUX_OSS:
		return _("OSS");
	case RtAudio::LINUX_PULSE:
		return _("PulseAudio");
	case RtAudio::MACOSX_CORE:
		return _("Core");
	case RtAudio::UNIX_JACK:
		return _("Jack");
	case RtAudio::WINDOWS_ASIO:
		return _("ASIO");
	case RtAudio::WINDOWS_DS:
		return _("DirectSound");
	case RtAudio::UNSPECIFIED:
	default:
		return _("Unknown");
	}
}

const wxChar* 
GOrgueRtHelpers::GetMidiApiPrefix(const RtMidi::Api api)
{
	switch (api)
	{
	case RtMidi::LINUX_ALSA:
	case RtMidi::MACOSX_CORE:
	case RtMidi::WINDOWS_MM:
		/* Old APIs include no prefix to stay backward compatible */
		return wxEmptyString;
	case RtMidi::UNIX_JACK:
		return _("Jack: ");
	case RtMidi::WINDOWS_KS:
		return _("KSAPI: ");
	case RtMidi::UNSPECIFIED:
	default:
		return _("Unknown: ");
	}
}
