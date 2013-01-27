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

#include "GOrgueConfigReader.h"
#include "GOrgueStop.h"
#include "GOrgueRank.h"
#include "GrandOrgueFile.h"

GOrgueStop::GOrgueStop(GrandOrgueFile* organfile, unsigned manual_number, unsigned first_midi_note_number) :
	GOrgueDrawstop(organfile),
	m_RankInfo(0),
	m_KeyVelocity(0),
	m_ManualNumber(manual_number),
	m_FirstMidiNoteNumber(first_midi_note_number),
	m_FirstAccessiblePipeLogicalKeyNumber(0),
	m_NumberOfAccessiblePipes(0)
{
}

unsigned GOrgueStop::IsAuto() const
{
	/* m_auto seems to state that if a stop only has 1 note, the note isn't
	 * actually controlled by a manual, but will be on if the stop is on and
	 * off if the stop is off... */
	return (m_RankInfo.size() == 1 && m_RankInfo[0].Rank->GetPipeCount() == 1);
}

void GOrgueStop::Load(GOrgueConfigReader& cfg, wxString group)
{
	unsigned number_of_ranks = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfRanks"), 0, m_organfile->GetRankCount(), false, 0);

	m_FirstAccessiblePipeLogicalKeyNumber  = cfg.ReadInteger(ODFSetting, group, wxT("FirstAccessiblePipeLogicalKeyNumber"), 1,  128);
	m_NumberOfAccessiblePipes              = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfAccessiblePipes"), 1, 192);

	if (number_of_ranks)
        {
		for(unsigned i = 0; i < number_of_ranks; i++)
		{
			RankInfo info;
			unsigned no = cfg.ReadInteger(ODFSetting, group, wxString::Format(wxT("Rank%03d"), i + 1), 1, m_organfile->GetRankCount());
			info.Rank = m_organfile->GetRank(no - 1); 
			info.FirstPipeNumber = cfg.ReadInteger(ODFSetting, group, wxString::Format(wxT("Rank%03dFirstPipeNumber"), i + 1), 1, info.Rank->GetPipeCount(), false, 1);
			info.PipeCount = cfg.ReadInteger(ODFSetting, group, wxString::Format(wxT("Rank%03dPipeCount"), i + 1), 1, info.Rank->GetPipeCount() - info.FirstPipeNumber + 1, false, info.Rank->GetPipeCount() - info.FirstPipeNumber + 1);
			info.FirstAccessibleKeyNumber = cfg.ReadInteger(ODFSetting, group, wxString::Format(wxT("Rank%03dFirstAccessibleKeyNumber"), i + 1), 1, m_NumberOfAccessiblePipes, false, 1);
			info.StopID = info.Rank->RegisterStop(this);
			m_RankInfo.push_back(info);
		}
        }
	else
	{
		RankInfo info;
		info.Rank = new GOrgueRank(m_organfile);
		m_organfile->AddRank(info.Rank);
		info.FirstPipeNumber = cfg.ReadInteger(ODFSetting, group, wxT("FirstAccessiblePipeLogicalPipeNumber"), 1, 192);
		info.FirstAccessibleKeyNumber = 1;
		info.PipeCount = m_NumberOfAccessiblePipes;
		info.Rank->Load(cfg, group, m_FirstMidiNoteNumber - info.FirstPipeNumber + info.FirstAccessibleKeyNumber + m_FirstAccessiblePipeLogicalKeyNumber - 1);
		info.StopID = info.Rank->RegisterStop(this);
		m_RankInfo.push_back(info);
	}

        m_KeyVelocity.resize(m_NumberOfAccessiblePipes);
        std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);

        GOrgueDrawstop::Load(cfg, group);
}

void GOrgueStop::SetRankKey(unsigned key, unsigned velocity)
{
	for(unsigned j = 0; j < m_RankInfo.size(); j++)
	{
		if (key + 1 < m_RankInfo[j].FirstAccessibleKeyNumber || key >= m_RankInfo[j].FirstAccessibleKeyNumber + m_RankInfo[j].PipeCount)
			continue;
		m_RankInfo[j].Rank->SetKey(key + m_RankInfo[j].FirstPipeNumber - m_RankInfo[j].FirstAccessibleKeyNumber, velocity, m_RankInfo[j].StopID);
	}
}

void GOrgueStop::SetKey(unsigned note, unsigned velocity)
{
	if (note < m_FirstAccessiblePipeLogicalKeyNumber || note >= m_FirstAccessiblePipeLogicalKeyNumber + m_NumberOfAccessiblePipes)
		return;
	if (IsAuto())
		return;
	note -= m_FirstAccessiblePipeLogicalKeyNumber;
	velocity = velocity >> 2;
	
	if (m_KeyVelocity[note] == velocity)
		return;
	m_KeyVelocity[note] = velocity;
	if (IsActive())
		SetRankKey(note, m_KeyVelocity[note]);
}

void GOrgueStop::ChangeState(bool on)
{
	if (IsAuto())
	{
		SetRankKey(0, on ? 0x7f : 0x00);
	}
	else
	{
		for(unsigned i = 0; i < m_NumberOfAccessiblePipes; i++)
			SetRankKey(i, on ? m_KeyVelocity[i] : 0);
	}

}

GOrgueStop::~GOrgueStop(void)
{
}

void GOrgueStop::Abort()
{
	if (IsAuto())
		Set(false);
	GOrgueButton::Abort();
}

void GOrgueStop::PreparePlayback()
{
	GOrgueDrawstop::PreparePlayback();

	m_KeyVelocity.resize(m_NumberOfAccessiblePipes);
	std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);

	if (IsAuto() && IsActive())
		SetRankKey(0, 0x7f);
}

GOrgueRank* GOrgueStop::GetRank(unsigned index)
{
	return m_RankInfo[index].Rank;
}
