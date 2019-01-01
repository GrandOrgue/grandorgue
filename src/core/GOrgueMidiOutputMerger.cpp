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

#include "GOrgueMidiOutputMerger.h"

#include "GOrgueRodgers.h"

GOrgueMidiOutputMerger::GOrgueMidiOutputMerger() :
	m_HWState(),
	m_RodgersState()
{
}

void GOrgueMidiOutputMerger::Clear()
{
	m_HWState.clear();
	m_RodgersState.clear();
}

bool GOrgueMidiOutputMerger::Process(GOrgueMidiEvent& e)
{
	if (e.GetMidiType() == MIDI_SYSEX_HW_STRING || e.GetMidiType() == MIDI_SYSEX_HW_LCD)
	{
		unsigned item = 0;
		while(item < m_HWState.size())
		{
			if (m_HWState[item].type == e.GetMidiType() &&
			    m_HWState[item].key == e.GetKey())
				break;
			item++;
		}
		if (item >= m_HWState.size())
		{
			GOrgueMidiOutputMergerHWState s;
			s.type = e.GetMidiType();
			s.key = e.GetKey();
			if (e.GetMidiType() == MIDI_SYSEX_HW_STRING)
				s.content = wxString(wxT(' '), 16);
			else
				s.content = wxString(wxT(' '), 32);
			m_HWState.push_back(s);
		}
		GOrgueMidiOutputMergerHWState& s = m_HWState[item];
		for(unsigned i = e.GetValue(), j = 0; i < s.content.length() && j < e.GetString().length(); i++, j++)
			s.content[i] = e.GetString()[j];
		e.SetString(s.content);
	}
	if (e.GetMidiType() == MIDI_SYSEX_RODGERS_STOP_CHANGE)
	{
		if ((unsigned)e.GetChannel() >= m_RodgersState.size())
			m_RodgersState.resize(e.GetChannel() + 1);
		std::vector<uint8_t>& data = m_RodgersState[e.GetChannel()];
		unsigned offset = GORodgersSetBit(e.GetKey(), e.GetValue(), data);
		std::vector<uint8_t> out(1);
		out[0] = data[offset];
		e.SetKey(offset);
		e.SetData(out);
	}
	return true;
}
