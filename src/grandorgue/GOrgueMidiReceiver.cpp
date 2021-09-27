/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
