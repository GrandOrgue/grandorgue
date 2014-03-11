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

#include "GOrgueMidiPlayer.h"

#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include "GOrgueMidiMerger.h"
#include "GOrgueMidiWXEvent.h"
#include "GOrgueMidiFileReader.h"
#include <wx/intl.h>

GOrgueMidiPlayer::GOrgueMidiPlayer(GOrgueMidi& midi) :
	wxTimer(),
	m_midi(midi),
	m_Events(),
	m_Pos(0),
	m_Last(0),
	m_Speed(1),
	m_IsPlaying(false)
{
	Clear();
}

GOrgueMidiPlayer::~GOrgueMidiPlayer()
{
	StopPlayer();
}

void GOrgueMidiPlayer::Clear()
{
	StopPlayer();
	m_Events.clear();
	m_Pos = 0;
}

void GOrgueMidiPlayer::ReadFileContent(GOrgueMidiFileReader& reader, std::vector<GOrgueMidiEvent>& events)
{
	unsigned pos = 0;
	GOrgueMidiEvent e;
	while(reader.ReadEvent(e))
	{
		while (pos > 0 && events[pos - 1].GetTime() > e.GetTime())
		{
			pos--;
		}
		while(pos < events.size() && events[pos].GetTime() < e.GetTime())
		{
			pos++;
		}
		if (pos <= events.size())
			events.insert(events.begin() + pos, e);
		else
			events.push_back(e);
		pos++;
	}
}

void GOrgueMidiPlayer::SetupManual(unsigned channel, wxString ID)
{
	GOrgueMidiEvent e;
	e.SetTime(0);

	e.SetMidiType(MIDI_SYSEX_GO_SETUP);
	e.SetKey(m_midi.GetMidiMap().GetElementByString(ID));
	e.SetChannel(channel);
	e.SetValue(0);
	m_Events.push_back(e);

	e.SetMidiType(MIDI_CTRL_CHANGE);
	e.SetChannel(channel);
	e.SetKey(MIDI_CTRL_NOTES_OFF);
	e.SetValue(0);
	m_Events.push_back(e);
}

void GOrgueMidiPlayer::Load(wxString filename, unsigned manuals, bool pedal)
{
	Clear();
	GOrgueMidiFileReader reader(m_midi.GetMidiMap());
	if (!reader.Open(filename))
	{
		GOMessageBox(wxString::Format(_("Failed to load %s"), filename.c_str()), _("MIDI Player"), wxOK | wxICON_ERROR, NULL);
		return;
	}
	std::vector<GOrgueMidiEvent> events;
	ReadFileContent(reader, events);

	GOrgueMidiMerger merger;
	merger.Clear();
	if (events.size() && events[0].GetMidiType() != MIDI_SYSEX_GO_CLEAR)
	{
		GOrgueMidiEvent e;
		e.SetTime(0);
		e.SetMidiType(MIDI_SYSEX_GO_CLEAR);
		m_Events.push_back(e);

		for(unsigned i = 1; i <= manuals; i++)
			SetupManual(i, wxString::Format(wxT("M%d"), i));
		if (pedal)
			SetupManual(manuals + 1, wxString::Format(wxT("M%d"), 0));
	}
	for(unsigned i = 0; i < events.size(); i++)
		if (merger.Process(events[i]))
			m_Events.push_back(events[i]);

	if (!reader.Close())
	{
		GOMessageBox(wxString::Format(_("Failed to decode %s"), filename.c_str()), _("MIDI Player"), wxOK | wxICON_ERROR, NULL);
		return;
	}
}

bool GOrgueMidiPlayer::IsLoaded()
{
	return m_Events.size() > 0;
}

void GOrgueMidiPlayer::Play()
{
	StopPlayer();
	m_DeviceID = m_midi.GetMidiMap().GetDeviceByString(_("GrandOrgue MIDI Player"));
	m_Pos = 0;
	m_Last = 0;
	m_IsPlaying = true;
	Notify();
}

void GOrgueMidiPlayer::StopPlayer()
{
	m_IsPlaying = false;
	wxTimer::Stop();
}

bool GOrgueMidiPlayer::IsPlaying()
{
	return m_IsPlaying;
}

void GOrgueMidiPlayer::Notify()
{
	if (!m_IsPlaying)
		return;
	do
	{
		if (m_Pos >= m_Events.size())
		{
			m_IsPlaying = false;
			return;
		}
		GOrgueMidiEvent e = m_Events[m_Pos];
		if (e.GetTime() <= m_Last)
		{
			m_Pos++;
			e.SetDevice(m_DeviceID);
			m_Last = e.GetTime();
			wxMidiEvent event(e);
			m_midi.AddPendingEvent(event);
		}
		else
		{
			GOTime wait = e.GetTime() - m_Last;
			m_Last = e.GetTime();
			wxTimer::Start(wait.GetValue(), true);
			return;
		}
	}
	while(true);
}
