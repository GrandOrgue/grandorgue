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

#include "GOrgueManual.h"

#include "GOrgueConfigReader.h"
#include "GOrgueCoupler.h"
#include "GOrgueDocument.h"
#include "GOrgueDivisional.h"
#include "GOrgueSettings.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueManual::GOrgueManual(GrandOrgueFile* organfile) :
	m_group(wxT("---")),
	m_midi(organfile, MIDI_RECV_MANUAL),
	m_sender(organfile, MIDI_SEND_MANUAL),
	m_division(organfile, MIDI_SEND_MANUAL),
	m_organfile(organfile),
	m_InputCouplers(),
	m_KeyVelocity(0),
	m_RemoteVelocity(),
	m_Velocity(),
	m_DivisionState(),
	m_Velocities(),
	m_manual_number(0),
	m_first_accessible_logical_key_nb(0),
	m_nb_logical_keys(0),
	m_first_accessible_key_midi_note_nb(0),
	m_nb_accessible_keys(0),
	m_UnisonOff(0),
	m_MIDIInputNumber(0),
	m_tremulant_ids(0),
	m_switch_ids(0),
	m_name(),
	m_stops(0),
	m_couplers(0),
	m_divisionals(0),
	m_displayed(false),
	m_DivisionalTemplate(organfile)
{
	m_InputCouplers.push_back(NULL);
	m_organfile->RegisterEventHandler(this);
	m_organfile->RegisterMidiConfigurator(this);
}

unsigned GOrgueManual::RegisterCoupler(GOrgueCoupler* coupler)
{
	m_InputCouplers.push_back(coupler);
	Resize();
	return m_InputCouplers.size() - 1;
}

void GOrgueManual::Resize()
{
	m_Velocity.resize(m_nb_logical_keys);
	m_DivisionState.resize(m_nb_logical_keys);
	m_RemoteVelocity.resize(m_nb_logical_keys);
	m_Velocities.resize(m_nb_logical_keys);
	for(unsigned i = 0; i < m_Velocities.size(); i++)
		m_Velocities[i].resize(m_InputCouplers.size());
}

void GOrgueManual::Init(GOrgueConfigReader& cfg, wxString group, int manualNumber, unsigned first_midi, unsigned keys)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_name = wxString::Format(_("Floating %d"), manualNumber - m_organfile->GetODFManualCount() + 1);
	m_nb_logical_keys = keys;
	m_first_accessible_logical_key_nb  = 1;
	m_first_accessible_key_midi_note_nb = first_midi;
	m_nb_accessible_keys = keys;
	m_MIDIInputNumber = 0;
	m_displayed = false;
	m_manual_number = manualNumber;

	m_stops.resize(0);
	m_couplers.resize(0);
	m_tremulant_ids.resize(0);
	m_switch_ids.resize(0);
	m_divisionals.resize(0);
	m_midi.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
	m_division.Load(cfg, group + wxT("Division"), m_organfile->GetSettings().GetMidiMap());

	SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("M%d"), m_manual_number)));

	Resize();
	m_KeyVelocity.resize(m_nb_accessible_keys);
	std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
}

void GOrgueManual::Load(GOrgueConfigReader& cfg, wxString group, int manualNumber)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_name                              = cfg.ReadString (ODFSetting, group, wxT("Name"));
	m_nb_logical_keys                   = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLogicalKeys"), 1, 192);
	m_first_accessible_logical_key_nb   = cfg.ReadInteger(ODFSetting, group, wxT("FirstAccessibleKeyLogicalKeyNumber"), 1, m_nb_logical_keys);
	m_first_accessible_key_midi_note_nb = cfg.ReadInteger(ODFSetting, group, wxT("FirstAccessibleKeyMIDINoteNumber"), 0, 127);
	m_nb_accessible_keys                = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfAccessibleKeys"), 0, 85);
	m_MIDIInputNumber                   = cfg.ReadInteger(ODFSetting, group, wxT("MIDIInputNumber"), 0, 200, false, 0);
	m_displayed                         = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"));
	unsigned nb_stops                 = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfStops"), 0, 999);
	unsigned nb_couplers              = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfCouplers"), 0, 999, false);
	unsigned nb_divisionals           = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfDivisionals"), 0, 999, false);
	unsigned nb_tremulants            = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount(), false);
	unsigned nb_switches              = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, m_organfile->GetSwitchCount(), false);
	m_manual_number = manualNumber;

	m_midi.SetIndex(manualNumber);

	for(unsigned i = 0; i < 128; i++)
		m_MidiMap[i] = cfg.ReadInteger(ODFSetting, group, wxString::Format(wxT("MIDIKey%03d"), i), 0, 127, false, i);

	wxString buffer;

	m_stops.resize(0);
	for (unsigned i = 0; i < nb_stops; i++)
	{
		m_stops.push_back(new GOrgueStop(m_organfile, m_manual_number, GetFirstLogicalKeyMIDINoteNumber()));
		buffer.Printf(wxT("Stop%03d"), i + 1);
		buffer.Printf(wxT("Stop%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 448));
		m_organfile->MarkSectionInUse(buffer);
		m_stops[i]->Load(cfg, buffer);
		m_stops[i]->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("M%dS%d"), m_manual_number, i)));
	}

	m_couplers.resize(0);
	for (unsigned i = 0; i < nb_couplers; i++)
	{
		m_couplers.push_back(new GOrgueCoupler(m_organfile, m_manual_number));
		buffer.Printf(wxT("Coupler%03d"), i + 1);
		buffer.Printf(wxT("Coupler%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 64));
		m_organfile->MarkSectionInUse(buffer);
		m_couplers[i]->Load(cfg, buffer);
		m_couplers[i]->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("M%dC%d"), m_manual_number, i)));
	}

	m_tremulant_ids.resize(0);
	for (unsigned i = 0; i < nb_tremulants; i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		unsigned new_id = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetTremulantCount());
		for(unsigned j = 0; j < m_tremulant_ids.size(); j++)
			if (m_tremulant_ids[j] == new_id)
				throw wxString::Format(_("Manual %d: Tremulant%03d already in use"), m_manual_number, new_id);
		m_tremulant_ids.push_back(new_id);
	}

	m_switch_ids.resize(0);
	for (unsigned i = 0; i < nb_switches; i++)
	{
		buffer.Printf(wxT("Switch%03d"), i + 1);
		unsigned new_id = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetSwitchCount());
		for(unsigned j = 0; j < m_switch_ids.size(); j++)
			if (m_switch_ids[j] == new_id)
				throw wxString::Format(_("Manual %d: Switch%03d already in use"), m_manual_number, new_id);
		m_switch_ids.push_back(new_id);
	}

	GetDivisionalTemplate().InitDivisional(m_manual_number);
	m_divisionals.resize(0);
	for (unsigned i = 0; i < nb_divisionals; i++)
	{
		m_divisionals.push_back(new GOrgueDivisional(m_organfile, GetDivisionalTemplate(), false));
		buffer.Printf(wxT("Divisional%03d"), i + 1);
		buffer.Printf(wxT("Divisional%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 224));
		m_organfile->MarkSectionInUse(buffer);
		m_divisionals[i]->Load(cfg, buffer, m_manual_number, i);
	}
	m_midi.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
	m_division.Load(cfg, group + wxT("Division"), m_organfile->GetSettings().GetMidiMap());

	SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("M%d"), m_manual_number)));

	Resize();
	m_KeyVelocity.resize(m_nb_accessible_keys);
	std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
}

void GOrgueManual::SetOutput(unsigned note, unsigned velocity)
{
	if (note < 0 || note >= m_DivisionState.size())
		return;
	velocity >>= 2;

	if (m_DivisionState[note] == velocity)
		return;
	m_DivisionState[note] = velocity;

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->SetKey(note + 1, velocity);

	int midi_note = note + m_first_accessible_key_midi_note_nb - m_first_accessible_logical_key_nb + 1;
	if (midi_note >= 0 && midi_note < 127)
		m_division.SetKey(midi_note, velocity);
}

void GOrgueManual::SetKey(unsigned note, unsigned velocity, GOrgueCoupler* prev, unsigned couplerID)
{
	if (note < 0 || note >= m_Velocity.size())
		return;

	if (m_Velocities[note][couplerID] == velocity)
		return;

	m_Velocities[note][couplerID] = velocity;
	m_Velocity[note] = m_Velocities[note][0];
	m_RemoteVelocity[note] = 0;
	for(unsigned i = 1; i < m_Velocities[note].size(); i++)
	{
		if (m_Velocity[note] < m_Velocities[note][i])
			m_Velocity[note] = m_Velocities[note][i];
		if (m_RemoteVelocity[note] < m_Velocities[note][i])
			m_RemoteVelocity[note] = m_Velocities[note][i];
	}

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->SetKey(note, m_Velocities[note], m_InputCouplers);

	SetOutput(note, m_UnisonOff > 0 ? m_RemoteVelocity[note] : m_Velocity[note]);

	if (m_first_accessible_logical_key_nb <= note + 1 && note <= m_first_accessible_logical_key_nb + m_nb_accessible_keys)
		m_organfile->ControlChanged(this);
}

void GOrgueManual::Set(unsigned note, unsigned velocity)
{
	if (note < m_first_accessible_key_midi_note_nb || note >= m_first_accessible_key_midi_note_nb + m_KeyVelocity.size())
		return;
	if (m_KeyVelocity[note - m_first_accessible_key_midi_note_nb] == velocity)
		return;
	m_KeyVelocity[note - m_first_accessible_key_midi_note_nb] = velocity;
	m_sender.SetKey(note, velocity);
	if (velocity)
		velocity = (velocity << 2) + 3;
	SetKey(note - m_first_accessible_key_midi_note_nb + m_first_accessible_logical_key_nb - 1, velocity, NULL, 0);
}

void GOrgueManual::SetUnisonOff(bool on)
{
	if (on)
	{
		if (m_UnisonOff++)
			return;
	}
	else
	{
		if (--m_UnisonOff)
			return;
	}
	for(unsigned note = 0; note < m_Velocity.size(); note++)
		SetOutput(note, on ? m_RemoteVelocity[note] : m_Velocity[note]);
}

GOrgueManual::~GOrgueManual(void)
{
}

GOrgueMidiReceiver& GOrgueManual::GetMidiReceiver()
{
	return m_midi;
}

GOrgueMidiSender& GOrgueManual::GetMidiSender()
{
	return m_sender;
}

const wxString& GOrgueManual::GetName()
{
	return m_name;
}

int GOrgueManual::GetMIDIInputNumber()
{
	return m_MIDIInputNumber;
}

unsigned GOrgueManual::GetLogicalKeyCount()
{
	return m_nb_logical_keys;
}

unsigned GOrgueManual::GetNumberOfAccessibleKeys()
{
	return m_nb_accessible_keys;
}

/* TODO: I suspect this could be made private or into something better... */
unsigned GOrgueManual::GetFirstAccessibleKeyMIDINoteNumber()
{
	return m_first_accessible_key_midi_note_nb;
}

int GOrgueManual::GetFirstLogicalKeyMIDINoteNumber()
{
	return m_first_accessible_key_midi_note_nb - m_first_accessible_logical_key_nb + 1;
}

unsigned GOrgueManual::GetStopCount()
{
	return m_stops.size();
}

GOrgueStop* GOrgueManual::GetStop(unsigned index)
{
	assert(index < m_stops.size());
	return m_stops[index];
}

unsigned GOrgueManual::GetCouplerCount()
{
	return m_couplers.size();
}

GOrgueCoupler* GOrgueManual::GetCoupler(unsigned index)
{
	assert(index < m_couplers.size());
	return m_couplers[index];
}

void GOrgueManual::AddCoupler(GOrgueCoupler* coupler)
{
	m_couplers.push_back(coupler);
}

unsigned GOrgueManual::GetDivisionalCount()
{
	return m_divisionals.size();
}

GOrgueDivisional* GOrgueManual::GetDivisional(unsigned index)
{
	assert(index < m_divisionals.size());
	return m_divisionals[index];
}

void GOrgueManual::AddDivisional(GOrgueDivisional* divisional)
{
	m_divisionals.push_back(divisional);
}

GOrgueCombinationDefinition& GOrgueManual::GetDivisionalTemplate()
{
	return m_DivisionalTemplate;
}

unsigned GOrgueManual::GetTremulantCount()
{
	return m_tremulant_ids.size();
}

GOrgueTremulant* GOrgueManual::GetTremulant(unsigned index)
{
	assert(index < m_tremulant_ids.size());
	return m_organfile->GetTremulant(m_tremulant_ids[index] - 1);
}

unsigned GOrgueManual::GetSwitchCount()
{
	return m_switch_ids.size();
}

GOrgueSwitch* GOrgueManual::GetSwitch(unsigned index)
{
	assert(index < m_switch_ids.size());
	return m_organfile->GetSwitch(m_switch_ids[index] - 1);
}

void GOrgueManual::AllNotesOff()
{
	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
		Set(m_first_accessible_key_midi_note_nb + j, 0x00);
}

bool GOrgueManual::IsKeyDown(unsigned midiNoteNumber)
{
	if (midiNoteNumber < m_first_accessible_key_midi_note_nb)
		return false;
	if (midiNoteNumber >= m_first_accessible_key_midi_note_nb + m_nb_accessible_keys)
		return false;
	if (midiNoteNumber - m_first_accessible_key_midi_note_nb + m_first_accessible_logical_key_nb - 1 >= m_Velocity.size())
		return false;
	return m_Velocity[midiNoteNumber - m_first_accessible_key_midi_note_nb + m_first_accessible_logical_key_nb - 1] > 0;
}

bool GOrgueManual::IsDisplayed()
{
	return m_displayed;
}

void GOrgueManual::Save(GOrgueConfigWriter& cfg)
{
	m_midi.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
	m_division.Save(cfg, m_group + wxT("Division"), m_organfile->GetSettings().GetMidiMap());
}

void GOrgueManual::Abort()
{
	AllNotesOff();

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->Abort();

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->Abort();

	for (unsigned i = 0; i < m_divisionals.size(); i++)
		m_divisionals[i]->Abort();
}

void GOrgueManual::PreparePlayback()
{
	m_midi.PreparePlayback();
	m_KeyVelocity.resize(m_nb_accessible_keys);
	std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
	m_sender.ResetKey();
	m_division.ResetKey();
	m_UnisonOff = 0;
	for(unsigned i = 0; i < m_Velocity.size(); i++)
		m_Velocity[i] = 0;
	for(unsigned i = 0; i < m_DivisionState.size(); i++)
		m_DivisionState[i] = 0;
	for(unsigned i = 0; i < m_RemoteVelocity.size(); i++)
		m_RemoteVelocity[i] = 0;
	for(unsigned i = 0; i < m_Velocities.size(); i++)
		for(unsigned j = 0; j < m_Velocities[i].size(); j++)
			m_Velocities[i][j] = 0;

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->PreparePlayback();

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->PreparePlayback();

	for (unsigned i = 0; i < m_divisionals.size(); i++)
		m_divisionals[i]->PreparePlayback();
}

void GOrgueManual::PrepareRecording()
{
	m_sender.ResetKey();
	for(unsigned i = 0; i < m_KeyVelocity.size(); i++)
		if (m_KeyVelocity[i] > 0)
			m_sender.SetKey(i + m_first_accessible_key_midi_note_nb, m_KeyVelocity[i]);

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->PrepareRecording();

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->PrepareRecording();
}

void GOrgueManual::Update()
{
	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->Update();

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->Update();
}

void GOrgueManual::ProcessMidi(const GOrgueMidiEvent& event)
{
	int key, value;

	switch(m_midi.Match(event, m_MidiMap, key, value))
	{
	case MIDI_MATCH_ON:
		if (value <= 0)
			value = 1;
		Set(key, value);
		break;

	case MIDI_MATCH_OFF:
		Set(key, 0x00);
		break;
		
	case MIDI_MATCH_RESET:
		AllNotesOff();
		break;
		
	default:
		break;
	}
}

void GOrgueManual::HandleKey(int key)
{
}

void GOrgueManual::Reset()
{
	for (unsigned j = 0; j < GetCouplerCount(); j++)
                GetCoupler(j)->Reset();
	for (unsigned j = 0; j < GetDivisionalCount(); j++)
		GetDivisional(j)->Display(false);

	if (GetStopCount() == 1 && !GetStop(0)->IsDisplayed())
		return;
	
	for (unsigned j = 0; j < GetStopCount(); j++)
                GetStop(j)->Reset();
}

void GOrgueManual::SetElementID(int id)
{
	m_midi.SetElementID(id);
	m_sender.SetElementID(id);
}

wxString GOrgueManual::GetMidiType()
{
	return _("Manual");
}

wxString GOrgueManual::GetMidiName()
{
	return GetName();
}

void GOrgueManual::ShowConfigDialog()
{
	wxString title = wxString::Format(_("Midi-Settings for %s - %s"), GetMidiType().c_str(), GetMidiName().c_str());

	m_organfile->GetDocument()->ShowMIDIEventDialog(this, title, &m_midi, &m_sender, NULL, &m_division);
}
