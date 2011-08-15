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

#include "GOrgueCoupler.h"
#include "IniFileConfig.h"
#include "GOrgueSound.h"
#include "GrandOrgueFile.h"

extern GrandOrgueFile* organfile;

GOrgueCoupler::GOrgueCoupler(unsigned sourceManual) :
	GOrgueDrawstop(),
	m_UnisonOff(false),
	m_CoupleToSubsequentUnisonIntermanualCouplers(false),
	m_CoupleToSubsequentUpwardIntermanualCouplers(false),
	m_CoupleToSubsequentDownwardIntermanualCouplers(false),
	m_CoupleToSubsequentUpwardIntramanualCouplers(false),
	m_CoupleToSubsequentDownwardIntramanualCouplers(false),
	m_CouplerType(COUPLER_NORMAL),
	m_SourceManual(sourceManual),
	m_DestinationManual(0),
	m_DestinationKeyshift(0),
	m_Keyshift(0),
	m_KeyState(0),
	m_InternalState(0),
	m_OutState(0),
	m_CurrentTone(-1),
	m_LastTone(-1)
{
}

void GOrgueCoupler::PreparePlayback()
{
	GOrgueManual* src = organfile->GetManual(m_SourceManual);
	GOrgueManual* dest = organfile->GetManual(m_DestinationManual);

	m_KeyState.resize(src->GetLogicalKeyCount());
	std::fill(m_KeyState.begin(), m_KeyState.end(), 0);
	m_InternalState.resize(dest->GetLogicalKeyCount());
	std::fill(m_InternalState.begin(), m_InternalState.end(), 0);
	m_OutState.resize(dest->GetLogicalKeyCount());
	std::fill(m_OutState.begin(), m_OutState.end(), 0);

	if (m_UnisonOff && DefaultToEngaged)
		src->SetUnisonOff(true);

	m_Keyshift = m_DestinationKeyshift + src->GetFirstLogicalKeyMIDINoteNumber() - dest->GetFirstLogicalKeyMIDINoteNumber();
}


void GOrgueCoupler::Load(IniFileConfig& cfg, wxString group, unsigned firstValidManualIndex, unsigned numberOfManuals, GOrgueDisplayMetrics* displayMetrics)
{

	m_UnisonOff                                     = cfg.ReadBoolean(group, wxT("UnisonOff"));
	m_DestinationManual                             = cfg.ReadInteger(group, wxT("DestinationManual"), firstValidManualIndex, numberOfManuals, !m_UnisonOff);
	m_DestinationKeyshift                           = cfg.ReadInteger(group, wxT("DestinationKeyshift"), -24, 24, !m_UnisonOff);
	m_CoupleToSubsequentUnisonIntermanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUnisonIntermanualCouplers"), !m_UnisonOff);
	m_CoupleToSubsequentUpwardIntermanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUpwardIntermanualCouplers"), !m_UnisonOff);
	m_CoupleToSubsequentDownwardIntermanualCouplers = cfg.ReadBoolean(group, wxT("CoupleToSubsequentDownwardIntermanualCouplers"), !m_UnisonOff);
	m_CoupleToSubsequentUpwardIntramanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUpwardIntramanualCouplers"), !m_UnisonOff);
	m_CoupleToSubsequentDownwardIntramanualCouplers = cfg.ReadBoolean(group, wxT("CoupleToSubsequentDownwardIntramanualCouplers"), !m_UnisonOff);
	GOrgueDrawstop::Load(cfg, group, displayMetrics);

	wxString type = cfg.ReadString(group, wxT("CouplerType"), 20, false);
	if (type == wxT("Bass"))
		m_CouplerType = COUPLER_BASS;
	else if (type == wxT("Melody"))
		m_CouplerType = COUPLER_MELODY;
	else if (type == wxT("Normal") || type == wxT(""))
		m_CouplerType = COUPLER_NORMAL;
	else
	{
		wxString error;
		error.Printf(_("Invalid coupler type for %s: '%s'"), group.c_str(), type.c_str());
		throw (wxString)error;
	}
}

void GOrgueCoupler::Save(IniFileConfig& cfg, bool prefix, wxString group)
{

	GOrgueDrawstop::Save(cfg, prefix, group);

}

void GOrgueCoupler::SetOut(int noteNumber, int on)
{
	if (noteNumber < 0)
		return;
	unsigned note = noteNumber;
	if (note >= m_InternalState.size())
		return;
	m_InternalState[note]+=on;

	if (!DefaultToEngaged)
		return;
	unsigned newstate = (m_InternalState[note] / 2);
	int change = newstate - m_OutState[note];
	GOrgueManual* dest = organfile->GetManual(m_DestinationManual);
	m_OutState[note] += change;
	dest->SetKey(note, change, this);
}

unsigned GOrgueCoupler::GetInternalState(int noteNumber)
{
	if (noteNumber < 0)
		return 0;
	unsigned note = noteNumber;
	if (note >= m_InternalState.size())
		return 0;
	return m_InternalState[note];
}


void GOrgueCoupler::ChangeKey(int note, int on)
{
	if (m_UnisonOff)
		return;
	if (m_CouplerType == COUPLER_BASS || m_CouplerType == COUPLER_MELODY)
	{
		int nextNote = -1;
		if (m_CouplerType == COUPLER_BASS)
		{
			for(nextNote = 0; nextNote< (int)m_KeyState.size(); nextNote++)
				if (m_KeyState[nextNote] > 1)
					break;
			if (nextNote == (int)m_KeyState.size())
				nextNote = -1;
		}
		else
		{
			for(nextNote=m_KeyState.size() - 1; nextNote >= 0; nextNote--)
				if (m_KeyState[nextNote] > 1)
					break;
		}

		if (m_CurrentTone != -1 && nextNote != m_CurrentTone)
			{
				SetOut(m_CurrentTone +  m_Keyshift, -GetInternalState(m_CurrentTone +  m_Keyshift));
				m_CurrentTone = -1;
			}

		if (((on > 1 && nextNote == note) || (on < 0 && nextNote == m_LastTone)))
			m_CurrentTone = nextNote;

		if (m_CurrentTone != -1)
			SetOut(m_CurrentTone +  m_Keyshift, m_KeyState[m_CurrentTone] - GetInternalState(m_CurrentTone + m_Keyshift));

		if (on > 1)
			m_LastTone = note;
		else
			m_LastTone = -1;
		return;
	}
	SetOut(note + m_Keyshift, on);
}


void GOrgueCoupler::SetKey(unsigned note, int on, GOrgueCoupler* prev)
{
	if (prev)
	{
		if ((prev->m_CoupleToSubsequentUnisonIntermanualCouplers && m_DestinationKeyshift == 0) ||
		    (prev->m_CoupleToSubsequentDownwardIntramanualCouplers && m_DestinationKeyshift < 0 && !IsIntermanual()) ||
		    (prev->m_CoupleToSubsequentUpwardIntramanualCouplers && m_DestinationKeyshift > 0 && !IsIntermanual()) ||
		    (prev->m_CoupleToSubsequentDownwardIntermanualCouplers &&m_DestinationKeyshift < 0 && IsIntermanual()) ||
		    (prev->m_CoupleToSubsequentUpwardIntermanualCouplers && m_DestinationKeyshift > 0 && IsIntermanual()))
			;
		else
			return;
	}
	if (note < 0 || note >= m_KeyState.size() || !on)
		return;
	m_KeyState[note]+=on;
	ChangeKey(note, on);
}

void GOrgueCoupler::Set(bool on)
{
	if (DefaultToEngaged == on)
		return;

	GOrgueDrawstop::Set(on);

	GOrgueManual* dest = organfile->GetManual(m_DestinationManual);

	if (m_UnisonOff)
		organfile->GetManual(m_SourceManual)->SetUnisonOff(on);

	for(unsigned i = 0; i < m_InternalState.size(); i++)
	{
		unsigned newstate = on ? (m_InternalState[i] / 2) : 0;
		if (m_OutState[i] != newstate)
		{
			int change = newstate - m_OutState[i];
			m_OutState[i] += change;
			dest->SetKey(i, change, this);
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
