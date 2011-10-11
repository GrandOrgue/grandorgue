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

#include "GOrguePushbutton.h"
#include "IniFileConfig.h"
#include "GrandOrgueFile.h"

GOrguePushbutton::GOrguePushbutton(GrandOrgueFile* organfile) :
	GOrgueControl(),
	m_midi(organfile, MIDI_RECV_BUTTON),
	m_organfile(organfile),
	m_IsPushed(false)
{
}

GOrgueMidiReceiver& GOrguePushbutton::GetMidiReceiver()
{
	return m_midi;
}

void GOrguePushbutton::Load(IniFileConfig& cfg, wxString group, wxString name)
{
	m_midi.Load(cfg, group);
	GOrgueControl::Load(cfg, group, name);
}

void GOrguePushbutton::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
}

void GOrguePushbutton::Display(bool on)
{
	if (m_IsPushed != on)
	{
		m_IsPushed = on;
		m_organfile->ControlChanged(this);
	}
}

void GOrguePushbutton::ProcessMidi(const GOrgueMidiEvent& event)
{
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
	case MIDI_MATCH_ON:
		Push();
		break;

	default:
		break;
	}
}

bool GOrguePushbutton::IsPushed()
{
	return m_IsPushed;
}
