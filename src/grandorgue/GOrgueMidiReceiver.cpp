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

#include "GOrgueMidiReceiver.h"

#include "GOrgueConfigReader.h"
#include "GOrgueEnclosure.h"
#include "GOrgueManual.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrgueMidiReceiver::GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type):
	GOrgueMidiReceiverBase(type),
	m_organfile(organfile),
	m_Index(-1)
{
}

void GOrgueMidiReceiver::SetIndex(int index)
{
	m_Index = index;
}
void GOrgueMidiReceiver::Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map)
{
	if (m_organfile && !m_organfile->GetSettings().ODFCheck())
	{
		/* Skip old style entries */
		if (m_type == MIDI_RECV_DRAWSTOP)
			cfg.ReadInteger(ODFSetting, group, wxT("StopControlMIDIKeyNumber"), -1, 127, false);
		if (m_type == MIDI_RECV_BUTTON)
			cfg.ReadInteger(ODFSetting, group, wxT("MIDIProgramChangeNumber"), 0, 128, false);
	}
	GOrgueMidiReceiverBase::Load(cfg, group, map);
}

void GOrgueMidiReceiver::Preconfigure(GOrgueConfigReader& cfg, wxString group)
{
	if (!m_organfile)
		return;
	unsigned index = 0;

	if (m_type == MIDI_RECV_SETTER)
	{
		index = m_Index;
	}
	if (m_type == MIDI_RECV_MANUAL)
	{
		if (m_Index == -1)
			return;

		index = m_organfile->GetManual(m_Index)->GetMIDIInputNumber();
	}
	if (m_type == MIDI_RECV_ENCLOSURE)
	{
		if (m_Index  == -1)
			return;

		index = m_organfile->GetEnclosureElement(m_Index)->GetMIDIInputNumber();
	}
	GOrgueMidiReceiverBase* recv = m_organfile->GetSettings().FindMidiEvent(m_type, index);
	if (!recv)
		return;

	for(unsigned i = 0; i < recv->GetEventCount(); i++)
		m_events.push_back(recv->GetEvent(i));
}

int GOrgueMidiReceiver::GetTranspose()
{
	return m_organfile->GetSettings().Transpose();
}

void GOrgueMidiReceiver::Assign(const GOrgueMidiReceiverData& data)
{
	GOrgueMidiReceiverBase::Assign(data);
	if (m_organfile)
		m_organfile->Modified();
}
