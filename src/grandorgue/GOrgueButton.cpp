/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueButton.h"

#include "GOrgueConfigReader.h"
#include "GrandOrgueFile.h"

GOrgueButton::GOrgueButton(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton) :
	m_organfile(organfile),
	m_midi(organfile, midi_type),
	m_sender(organfile, MIDI_SEND_BUTTON),
	m_shortcut(organfile),
	m_Pushbutton(pushbutton),
	m_group(wxT("---")),
	m_Displayed(false),
	m_Name(),
	m_Engaged(false),
	m_DisplayInInvertedState(false),
	m_ReadOnly(false)
{
}

GOrgueButton::~GOrgueButton()
{
}

void GOrgueButton::Init(GOrgueConfigReader& cfg, wxString group, wxString name)
{
	m_group = group;
	m_Name = name;
	m_Displayed = false;
	m_DisplayInInvertedState = false;
	if (!m_ReadOnly)
	{
		m_midi.Load(cfg, group);
		m_shortcut.Load(cfg, group);
	}
	m_sender.Load(cfg, group);
}

void GOrgueButton::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_group = group;
	m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"), 64, true);
	m_Displayed = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), true, false);
	m_DisplayInInvertedState = cfg.ReadBoolean(ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
	if (!m_ReadOnly)
	{
		m_midi.Load(cfg, group);
		m_shortcut.Load(cfg, group);
	}
	m_sender.Load(cfg, group);
}

void GOrgueButton::Save(GOrgueConfigWriter& cfg)
{
	if (!m_ReadOnly)
	{
		m_midi.Save(cfg, m_group);
		m_shortcut.Save(cfg, m_group);
	}
	m_sender.Save(cfg, m_group);
}

bool GOrgueButton::IsDisplayed()
{
	return m_Displayed;
}

bool GOrgueButton::IsReadOnly()
{
	return m_ReadOnly;
}

const wxString& GOrgueButton::GetName()
{
	return m_Name;
}

GOrgueMidiReceiver& GOrgueButton::GetMidiReceiver()
{
	return m_midi;
}

GOrgueMidiSender& GOrgueButton::GetMidiSender()
{
	return m_sender;
}

GOrgueKeyReceiver& GOrgueButton::GetKeyReceiver()
{
	return m_shortcut;
}

void GOrgueButton::HandleKey(int key)
{
	if (m_ReadOnly)
		return;
	if (m_shortcut.Match(key))
		Push();
}

void GOrgueButton::Push()
{
	if (m_ReadOnly)
		return;
	Set(m_Engaged ^ true);
}

void GOrgueButton::Set(bool on)
{
}

void GOrgueButton::Abort()
{
	m_sender.SetDisplay(false);
}

void GOrgueButton::PreparePlayback()
{
	m_sender.SetDisplay(m_Engaged);
}

void GOrgueButton::ProcessMidi(const GOrgueMidiEvent& event)
{
	if (m_ReadOnly)
		return;
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
	m_sender.SetDisplay(onoff);
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
