/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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

#include <wx/app.h>
#include "GOrgueEnclosure.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

GOrgueEnclosure::GOrgueEnclosure(GrandOrgueFile* organfile) :
	m_group(wxT("---")),
	m_midi(organfile, MIDI_RECV_ENCLOSURE),
	m_organfile(organfile),
	m_AmpMinimumLevel(0),
	m_MIDIInputNumber(0),
	m_MIDIValue(0),
	m_Name()
{

}
GOrgueEnclosure::~GOrgueEnclosure()
{
}

void GOrgueEnclosure::Init(IniFileConfig& cfg, wxString group, wxString Name)
{
	m_group = group;
	m_Name = cfg.ReadString(m_group, wxT("Name"), 64, false, Name);
	Set(0);	// default to down
	m_midi.Load(cfg, m_group);
}

void GOrgueEnclosure::Load(IniFileConfig& cfg, wxString group, int enclosure_nb)
{
	m_group = group;
	m_Name = cfg.ReadString(m_group, wxT("Name"), 64);
	m_AmpMinimumLevel = cfg.ReadInteger(m_group, wxT("AmpMinimumLevel"), 0, 100);
	m_MIDIInputNumber = cfg.ReadInteger(m_group, wxT("MIDIInputNumber"), 1, 6);
	Set(127);	// default to full volume until we receive any messages
	m_midi.SetIndex(enclosure_nb);
	m_midi.Load(cfg, m_group);
}

void GOrgueEnclosure::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
}

void GOrgueEnclosure::Set(int n)
{
	if (n < 0)
		n = 0;
	if (n > 127)
		n = 127;
	m_MIDIValue = n;
	m_organfile->ControlChanged(this);
}

int GOrgueEnclosure::GetMIDIInputNumber()
{
	return m_MIDIInputNumber;
}

float GOrgueEnclosure::GetAttenuation()
{
	static const float scale = 1.0 / 12700.0;
	return (float)(m_MIDIValue * (100 - m_AmpMinimumLevel) + 127 * m_AmpMinimumLevel) * scale;
}

void GOrgueEnclosure::Scroll(bool scroll_up)
{
	Set(m_MIDIValue + (scroll_up ? 4 : -4));
}

void GOrgueEnclosure::ProcessMidi(const GOrgueMidiEvent& event)
{
	int value;
	if (m_midi.Match(event, value) == MIDI_MATCH_CHANGE)
		Set(value);
}

GOrgueMidiReceiver& GOrgueEnclosure::GetMidiReceiver()
{
	return m_midi;
}

const wxString& GOrgueEnclosure::GetName()
{
	return m_Name;
}

int GOrgueEnclosure::GetValue()
{
	return m_MIDIValue;
}
