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

#include "GOrgueSetterButton.h"
#include "GOrgueSetter.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

GOrgueSetterButton::GOrgueSetterButton(GrandOrgueFile* organfile, GOrgueSetter* setter, bool Pushbutton) :
	m_midi(organfile, MIDI_RECV_SETTER),
	m_organfile(organfile),
	m_setter(setter),
	m_DefaultToEngaged(false),
	m_Pushbutton(Pushbutton),
	m_Name(),
	m_group()
{
}

GOrgueSetterButton::~GOrgueSetterButton()
{

}

void GOrgueSetterButton::Load(IniFileConfig& cfg, wxString group, wxString Name)
{
	m_group = group;
	m_Name = cfg.ReadString(group, wxT("Name"), 255, false, Name);
	m_midi.Load(cfg, group);
}

void GOrgueSetterButton::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
}

void GOrgueSetterButton::Push()
{
	if (m_Pushbutton)
		m_setter->Change(this);
	else
		Set(m_DefaultToEngaged ^ true);
}

GOrgueMidiReceiver& GOrgueSetterButton::GetMidiReceiver()
{
	return m_midi;
}

void GOrgueSetterButton::Display(bool on)
{
	m_DefaultToEngaged = on;
	m_organfile->ControlChanged(this);
}

void GOrgueSetterButton::Set(bool on)
{
	if (m_DefaultToEngaged == on)
		return;
	m_setter->Change(this);
	m_DefaultToEngaged = on;
	m_organfile->ControlChanged(this);
}

void GOrgueSetterButton::ProcessMidi(const GOrgueMidiEvent& event)
{
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
		Push();
		break;

	case MIDI_MATCH_ON:
		if (m_Pushbutton)
			Push();
		else
			Set(true);
		break;

	case MIDI_MATCH_OFF:
		if (!m_Pushbutton)
			Set(false);
		break;

	default:
		break;
	}
}

bool GOrgueSetterButton::IsEngaged() const
{
	return m_DefaultToEngaged;
}


const wxString& GOrgueSetterButton::GetName()
{
	return m_Name;
}
