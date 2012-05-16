/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOrgueButton.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

GOrgueButton::GOrgueButton(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton) :
	m_organfile(organfile),
	m_midi(organfile, midi_type),
	m_Pushbutton(pushbutton),
	m_group(wxT("---")),
	m_Displayed(false),
	m_Name(),
	m_Engaged(false),
	m_DisplayInInvertedState(false)
{
}

GOrgueButton::~GOrgueButton()
{
}

void GOrgueButton::Load(IniFileConfig& cfg, wxString group, wxString name)
{
	m_group = group;
	m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"), 64, true, name);
	m_Displayed = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), true, false);
	m_DisplayInInvertedState = cfg.ReadBoolean(ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
	m_midi.Load(cfg, group);
}

void GOrgueButton::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
}

bool GOrgueButton::IsDisplayed()
{
	return m_Displayed;
}

const wxString& GOrgueButton::GetName()
{
	return m_Name;
}

GOrgueMidiReceiver& GOrgueButton::GetMidiReceiver()
{
	return m_midi;
}

void GOrgueButton::Push()
{
	Set(m_Engaged ^ true);
}

void GOrgueButton::Set(bool on)
{
}

void GOrgueButton::ProcessMidi(const GOrgueMidiEvent& event)
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

void GOrgueButton::Display(bool onoff)
{
	if (m_Engaged == onoff)
		return;
	m_Engaged = onoff;
	m_organfile->ControlChanged(this);
}

bool GOrgueButton::IsEngaged() const
{
	return m_Engaged;
}

bool GOrgueButton::DisplayInverted() const
{
	return m_DisplayInInvertedState;
}
