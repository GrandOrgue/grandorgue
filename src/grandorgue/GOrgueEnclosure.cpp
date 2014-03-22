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

#include "GOrgueEnclosure.h"

#include "GOrgueConfigReader.h"
#include "GOrgueDocument.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueEnclosure::GOrgueEnclosure(GrandOrgueFile* organfile) :
	m_midi(organfile, MIDI_RECV_ENCLOSURE),
	m_sender(organfile, MIDI_SEND_ENCLOSURE),
	m_organfile(organfile),
	m_AmpMinimumLevel(0),
	m_MIDIInputNumber(0),
	m_MIDIValue(0),
	m_Name(),
	m_Displayed(false)
{
	m_organfile->RegisterEventHandler(this);
	m_organfile->RegisterMidiConfigurator(this);
}

GOrgueEnclosure::~GOrgueEnclosure()
{
}

void GOrgueEnclosure::Init(GOrgueConfigReader& cfg, wxString group, wxString Name)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_Name = cfg.ReadString(ODFSetting, m_group, wxT("Name"), false, Name);
	Set(0);	// default to down
	m_midi.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_AmpMinimumLevel = 0;
}

void GOrgueEnclosure::Load(GOrgueConfigReader& cfg, wxString group, int enclosure_nb)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_Name = cfg.ReadString(ODFSetting, m_group, wxT("Name"));
	m_Displayed = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, true);
	m_AmpMinimumLevel = cfg.ReadInteger(ODFSetting, m_group, wxT("AmpMinimumLevel"), 0, 100);
	m_MIDIInputNumber = cfg.ReadInteger(ODFSetting, m_group, wxT("MIDIInputNumber"), 0, 200, false, 0);
	Set(127);	// default to full volume until we receive any messages
	m_midi.SetIndex(enclosure_nb);
	m_midi.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOrgueEnclosure::Save(GOrgueConfigWriter& cfg)
{
	m_midi.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOrgueEnclosure::Set(int n)
{
	if (n < 0)
		n = 0;
	if (n > 127)
		n = 127;
	m_MIDIValue = n;
	m_sender.SetValue(m_MIDIValue);
	m_organfile->UpdateVolume();
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

void GOrgueEnclosure::HandleKey(int key)
{
}

GOrgueMidiReceiver& GOrgueEnclosure::GetMidiReceiver()
{
	return m_midi;
}

GOrgueMidiSender& GOrgueEnclosure::GetMidiSender()
{
	return m_sender;
}

const wxString& GOrgueEnclosure::GetName()
{
	return m_Name;
}

int GOrgueEnclosure::GetValue()
{
	return m_MIDIValue;
}

bool GOrgueEnclosure::IsDisplayed()
{
	return m_Displayed;
}


void GOrgueEnclosure::Abort()
{
	m_sender.SetValue(0);
}

void GOrgueEnclosure::PreparePlayback()
{
	m_midi.PreparePlayback();
	m_sender.SetValue(m_MIDIValue);
}

void GOrgueEnclosure::PrepareRecording()
{
	m_sender.SetValue(m_MIDIValue);
}

void GOrgueEnclosure::SetElementID(int id)
{
	m_midi.SetElementID(id);
	m_sender.SetElementID(id);
}

wxString GOrgueEnclosure::GetMidiType()
{
	return _("Enclosure");
}

wxString GOrgueEnclosure::GetMidiName()
{
	return GetName();
}

void GOrgueEnclosure::ShowConfigDialog()
{
	wxString title = wxString::Format(_("Midi-Settings for %s - %s"), GetMidiType().c_str(), GetMidiName().c_str());

	m_organfile->GetDocument()->ShowMIDIEventDialog(this, title, &m_midi, &m_sender, NULL);
}
