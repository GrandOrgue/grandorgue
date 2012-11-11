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
 */

#include "GOrgueConfigReader.h"
#include "GOrgueCoupler.h"
#include "GOrgueManual.h"
#include "GrandOrgueFile.h"

GOrgueCoupler::GOrgueCoupler(GrandOrgueFile* organfile, unsigned sourceManual) :
	GOrgueDrawstop(organfile),
	m_UnisonOff(false),
	m_CoupleToSubsequentUnisonIntermanualCouplers(false),
	m_CoupleToSubsequentUpwardIntermanualCouplers(false),
	m_CoupleToSubsequentDownwardIntermanualCouplers(false),
	m_CoupleToSubsequentUpwardIntramanualCouplers(false),
	m_CoupleToSubsequentDownwardIntramanualCouplers(false),
	m_CouplerType(COUPLER_NORMAL),
	m_SourceManual(sourceManual),
	m_CouplerID(0),
	m_DestinationManual(0),
	m_DestinationKeyshift(0),
	m_Keyshift(0),
	m_KeyVelocity(0),
	m_InternalVelocity(0),
	m_OutVelocity(0),
	m_CurrentTone(-1),
	m_LastTone(-1),
	m_FirstMidiNote(0),
	m_FirstLogicalKey(0),
	m_NumberOfKeys(127)
{
}

void GOrgueCoupler::PreparePlayback()
{
	GOrgueDrawstop::PreparePlayback();

	GOrgueManual* src = m_organfile->GetManual(m_SourceManual);
	GOrgueManual* dest = m_organfile->GetManual(m_DestinationManual);

	m_KeyVelocity.resize(src->GetLogicalKeyCount());
	std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);
	m_InternalVelocity.resize(dest->GetLogicalKeyCount());
	std::fill(m_InternalVelocity.begin(), m_InternalVelocity.end(), 0);
	m_OutVelocity.resize(dest->GetLogicalKeyCount());
	std::fill(m_OutVelocity.begin(), m_OutVelocity.end(), 0);

	if (m_UnisonOff && IsActive())
		src->SetUnisonOff(true);

	m_Keyshift = m_DestinationKeyshift + src->GetFirstLogicalKeyMIDINoteNumber() - dest->GetFirstLogicalKeyMIDINoteNumber();
	if (m_FirstMidiNote > src->GetFirstLogicalKeyMIDINoteNumber())
		m_FirstLogicalKey = m_FirstMidiNote - src->GetFirstLogicalKeyMIDINoteNumber();
	else
		m_FirstLogicalKey = 0;
}

const struct IniFileEnumEntry GOrgueCoupler::m_coupler_types[]={
	{ wxT("Normal"), COUPLER_NORMAL},
	{ wxT("Bass"), COUPLER_BASS},
	{ wxT("Melody"), COUPLER_MELODY},
};


void GOrgueCoupler::Load(GOrgueConfigReader& cfg, wxString group, wxString name, bool unison_off, bool recursive, int keyshift, int dest_manual, GOrgueCouplerType coupler_type)
{

	m_UnisonOff                                     = cfg.ReadBoolean(ODFSetting, group, wxT("UnisonOff"), true, unison_off);
	m_DestinationManual                             = cfg.ReadInteger(ODFSetting, group, wxT("DestinationManual"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount(), !m_UnisonOff, dest_manual);
	m_DestinationKeyshift                           = cfg.ReadInteger(ODFSetting, group, wxT("DestinationKeyshift"), -24, 24, !m_UnisonOff, keyshift);
	m_CoupleToSubsequentUnisonIntermanualCouplers   = cfg.ReadBoolean(ODFSetting, group, wxT("CoupleToSubsequentUnisonIntermanualCouplers"), !m_UnisonOff, recursive);
	m_CoupleToSubsequentUpwardIntermanualCouplers   = cfg.ReadBoolean(ODFSetting, group, wxT("CoupleToSubsequentUpwardIntermanualCouplers"), !m_UnisonOff, recursive);
	m_CoupleToSubsequentDownwardIntermanualCouplers = cfg.ReadBoolean(ODFSetting, group, wxT("CoupleToSubsequentDownwardIntermanualCouplers"), !m_UnisonOff, recursive);
	m_CoupleToSubsequentUpwardIntramanualCouplers   = cfg.ReadBoolean(ODFSetting, group, wxT("CoupleToSubsequentUpwardIntramanualCouplers"), !m_UnisonOff, recursive);
	m_CoupleToSubsequentDownwardIntramanualCouplers = cfg.ReadBoolean(ODFSetting, group, wxT("CoupleToSubsequentDownwardIntramanualCouplers"), !m_UnisonOff, recursive);
	GOrgueDrawstop::Load(cfg, group, name);

	m_CouplerType = (GOrgueCouplerType)cfg.ReadEnum(ODFSetting, group, wxT("CouplerType"), m_coupler_types, sizeof(m_coupler_types) / sizeof(m_coupler_types[0]), false, coupler_type);
	m_FirstMidiNote = cfg.ReadInteger(ODFSetting, group, wxT("FirstMIDINoteNumber"), 0, 127, false, 0); 
	m_NumberOfKeys = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfKeys"), 0, 127, false, 127);

	if (!m_UnisonOff)
		m_CouplerID = m_organfile->GetManual(m_DestinationManual)->RegisterCoupler(this);
}

void GOrgueCoupler::Save(GOrgueConfigWriter& cfg)
{
	GOrgueDrawstop::Save(cfg);
}

void GOrgueCoupler::SetOut(int noteNumber, unsigned velocity)
{
	if (noteNumber < 0)
		return;
	unsigned note = noteNumber;
	if (note >= m_InternalVelocity.size())
		return;
	if (m_InternalVelocity[note] == velocity)
		return;
	m_InternalVelocity[note] = velocity;

	if (!IsActive())
		return;
	unsigned newstate = m_InternalVelocity[note];
	if (newstate)
		newstate--;
	GOrgueManual* dest = m_organfile->GetManual(m_DestinationManual);
	m_OutVelocity[note] = newstate;
	dest->SetKey(note, m_OutVelocity[note], this, m_CouplerID);
}

unsigned GOrgueCoupler::GetInternalState(int noteNumber)
{
	if (noteNumber < 0)
		return 0;
	unsigned note = noteNumber;
	if (note >= m_InternalVelocity.size())
		return 0;
	return m_InternalVelocity[note];
}

void GOrgueCoupler::ChangeKey(int note, unsigned velocity)
{
	if (m_UnisonOff)
		return;
	if (m_CouplerType == COUPLER_BASS || m_CouplerType == COUPLER_MELODY)
	{
		int nextNote = -1;
		if (m_CouplerType == COUPLER_BASS)
		{
			for(nextNote = 0; nextNote < (int)m_KeyVelocity.size(); nextNote++)
				if (m_KeyVelocity[nextNote] > 0)
					break;
			if (nextNote == (int)m_KeyVelocity.size())
				nextNote = -1;
		}
		else
		{
			for(nextNote = m_KeyVelocity.size() - 1; nextNote >= 0; nextNote--)
				if (m_KeyVelocity[nextNote] > 0)
					break;
		}

		if (m_CurrentTone != -1 && nextNote != m_CurrentTone)
			{
				SetOut(m_CurrentTone +  m_Keyshift, 0);
				m_CurrentTone = -1;
			}

		if (((velocity > 0 && nextNote == note) || (velocity == 0 && nextNote == m_LastTone)))
			m_CurrentTone = nextNote;

		if (m_CurrentTone != -1)
			SetOut(m_CurrentTone +  m_Keyshift, velocity);

		if (velocity > 0)
			m_LastTone = note;
		else
			m_LastTone = -1;
		return;
	}
	SetOut(note + m_Keyshift, velocity);
}

void GOrgueCoupler::SetKey(unsigned note, const std::vector<unsigned>& velocities, const std::vector<GOrgueCoupler*>& couplers)
{
	if (note < 0 || note >= m_KeyVelocity.size())
		return;
	if (note < m_FirstLogicalKey || note >= m_FirstLogicalKey + m_NumberOfKeys)
		return;

	assert(velocities.size() == couplers.size());
	unsigned velocity = 0;
	for(unsigned i = 0; i < velocities.size(); i++)
	{
		GOrgueCoupler* prev = couplers[i];
		if (prev)
		{
			if ((prev->m_CoupleToSubsequentUnisonIntermanualCouplers && m_DestinationKeyshift == 0) ||
			    (prev->m_CoupleToSubsequentDownwardIntramanualCouplers && m_DestinationKeyshift < 0 && !IsIntermanual()) ||
			    (prev->m_CoupleToSubsequentUpwardIntramanualCouplers && m_DestinationKeyshift > 0 && !IsIntermanual()) ||
			    (prev->m_CoupleToSubsequentDownwardIntermanualCouplers &&m_DestinationKeyshift < 0 && IsIntermanual()) ||
			    (prev->m_CoupleToSubsequentUpwardIntermanualCouplers && m_DestinationKeyshift > 0 && IsIntermanual()))
				;
			else
				continue;
		}
		if (velocities[i] > velocity)
			velocity = velocities[i];
	}
	if (m_KeyVelocity[note] == velocity)
		return;
	m_KeyVelocity[note] = velocity;
	ChangeKey(note, velocity);
}

void GOrgueCoupler::ChangeState(bool on)
{
	GOrgueManual* dest = m_organfile->GetManual(m_DestinationManual);

	if (m_UnisonOff)
	{
		m_organfile->GetManual(m_SourceManual)->SetUnisonOff(on);
		return;
	}

	for(unsigned i = 0; i < m_InternalVelocity.size(); i++)
	{
		unsigned newstate = on ? m_InternalVelocity[i] : 0;
		if (newstate > 0)
			newstate--;
		if (m_OutVelocity[i] != newstate)
		{
			m_OutVelocity[i] = newstate;
			dest->SetKey(i, m_OutVelocity[i], this, m_CouplerID);
		}
	}
}

bool GOrgueCoupler::IsIntermanual()
{
	return m_SourceManual != m_DestinationManual;
}

bool GOrgueCoupler::IsUnisonOff()
{
	return m_UnisonOff;
}
