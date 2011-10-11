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

#include "GOrgueDrawStop.h"
#include "GOrgueLCD.h"
#include "GrandOrgueFile.h"

GOrgueDrawstop::GOrgueDrawstop(GrandOrgueFile* organfile) :
	GOrgueControl(),
	m_midi(organfile, MIDI_RECV_DRAWSTOP),
	m_organfile(organfile),
	m_DefaultToEngaged(false),
	m_DisplayInInvertedState(false)
{

}

GOrgueDrawstop::~GOrgueDrawstop()
{

}

void GOrgueDrawstop::Load(IniFileConfig& cfg, wxString group, wxString name)
{
	m_DefaultToEngaged = cfg.ReadBoolean(group, wxT("DefaultToEngaged"));
	m_DisplayInInvertedState = cfg.ReadBoolean(group, wxT("DisplayInInvertedState"));
	m_midi.Load(cfg, group);
	GOrgueControl::Load(cfg, group, name);
}

void GOrgueDrawstop::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
	cfg.SaveHelper(prefix, m_group, wxT("DefaultToEngaged"), m_DefaultToEngaged ? wxT("Y") : wxT("N"));
	GOrgueControl::Save(cfg, prefix);
}

void GOrgueDrawstop::Push()
{
	Set(m_DefaultToEngaged ^ true);
}

GOrgueMidiReceiver& GOrgueDrawstop::GetMidiReceiver()
{
	return m_midi;
}

void GOrgueDrawstop::Set(bool on)
{
	if (m_DefaultToEngaged == on)
		return;
	m_DefaultToEngaged = on;
	m_organfile->ControlChanged(this);
	GOrgueLCD_WriteLineTwo(GetName(), 2000);
}

void GOrgueDrawstop::ProcessMidi(const GOrgueMidiEvent& event)
{
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
		Push();
		break;

	case MIDI_MATCH_ON:
		Set(true);
		break;

	case MIDI_MATCH_OFF:
		Set(false);
		break;

	default:
		break;
	}
}

bool GOrgueDrawstop::IsEngaged() const
{
	return m_DefaultToEngaged;
}

bool GOrgueDrawstop::DisplayInverted() const
{
	return m_DisplayInInvertedState;
}
