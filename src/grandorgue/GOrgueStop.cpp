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

#include "GOrgueStop.h"
#include "GOrguePipe.h"
#include "GrandOrgueFile.h"

GOrgueStop::GOrgueStop(GrandOrgueFile* organfile, unsigned manual_number) :
	GOrgueDrawstop(organfile),
	m_Pipes(0),
	m_KeyState(0),
	m_ManualNumber(manual_number),
	m_Percussive(false),
	m_AmplitudeLevel(0),
	m_FirstAccessiblePipeLogicalPipeNumber(0),
	m_FirstAccessiblePipeLogicalKeyNumber(0),
	m_NumberOfAccessiblePipes(0),
	m_WindchestGroup(0)
{
}

unsigned GOrgueStop::GetFirstAccessiblePipeLogicalPipeNumber() const
{
	return m_FirstAccessiblePipeLogicalPipeNumber;
}

unsigned GOrgueStop::GetFirstAccessiblePipeLogicalKeyNumber() const
{
	return m_FirstAccessiblePipeLogicalKeyNumber;
}

unsigned GOrgueStop::GetNbAccessiblePipes() const
{
	return
		IsAuto()
		? 0 /* When there is only one logical pipe, the pipe plays whenever
		     * the stop is on and we "fake" that the stop has no accessible
		     * pipes. */
		: m_NumberOfAccessiblePipes;
}

unsigned GOrgueStop::IsAuto() const
{
	/* m_auto seems to state that if a stop only has 1 note, the note isn't
	 * actually controlled by a manual, but will be on if the stop is on and
	 * off if the stop is off... */
	return (m_Pipes.size() == 1);
}

unsigned GOrgueStop::GetAmplitude() const
{
	return m_AmplitudeLevel;
}

void GOrgueStop::Load(IniFileConfig& cfg, wxString group)
{

	unsigned number_of_logical_pipes       = cfg.ReadInteger(group, wxT("NumberOfLogicalPipes"), 1, 192);
	m_AmplitudeLevel                       = cfg.ReadInteger(group, wxT("AmplitudeLevel"), 0, 1000);
	m_FirstAccessiblePipeLogicalPipeNumber = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalPipeNumber"), 1, number_of_logical_pipes);
	m_FirstAccessiblePipeLogicalKeyNumber  = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalKeyNumber"), 1,  128);
	m_NumberOfAccessiblePipes              = cfg.ReadInteger(group, wxT("NumberOfAccessiblePipes"), 1, number_of_logical_pipes);
	m_WindchestGroup                       = cfg.ReadInteger(group, wxT("WindchestGroup"), 1, m_organfile->GetWinchestGroupCount());
	m_Percussive                           = cfg.ReadBoolean(group, wxT("Percussive"));

	m_Pipes.clear();
	for (unsigned i = 0; i < number_of_logical_pipes; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Pipe%03u"), i + 1);
		wxString filename = cfg.ReadString(group, buffer);
		m_Pipes.push_back
			(new GOrguePipe
				(m_organfile
				,filename
				,m_Percussive
				,m_WindchestGroup
				,m_organfile->GetAmplitude() * m_AmplitudeLevel
				)
			);
	}
	m_KeyState.resize(m_NumberOfAccessiblePipes);
	std::fill(m_KeyState.begin(), m_KeyState.end(), 0);

	GOrgueDrawstop::Load(cfg, group);

}

void GOrgueStop::Save(IniFileConfig& cfg, bool prefix)
{
	GOrgueDrawstop::Save(cfg, prefix);
}

void GOrgueStop::SetKey(unsigned note, int on)
{
	if (note < m_FirstAccessiblePipeLogicalKeyNumber || note >= m_FirstAccessiblePipeLogicalKeyNumber + m_NumberOfAccessiblePipes)
		return;
	note -= m_FirstAccessiblePipeLogicalKeyNumber;
	
	unsigned last = m_KeyState[note];
	m_KeyState[note] += on;

	if (IsEngaged())
	{
		if (last > 0 && m_KeyState[note] == 0)
			m_Pipes[note + m_FirstAccessiblePipeLogicalPipeNumber - 1]->Set(false);
		if (last == 0 && m_KeyState[note] > 0)
			m_Pipes[note + m_FirstAccessiblePipeLogicalPipeNumber - 1]->Set(true);
	}
}


void GOrgueStop::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	for(unsigned i = 0; i < m_NumberOfAccessiblePipes; i++)
		if (m_KeyState[i])
			m_Pipes[i + m_FirstAccessiblePipeLogicalPipeNumber - 1]->Set(on);

	GOrgueDrawstop::Set(on);

	if (IsAuto())
		m_Pipes[0]->Set(on);
}

GOrguePipe* GOrgueStop::GetPipe(unsigned index)
{
	return m_Pipes[index];
}

GOrgueStop::~GOrgueStop(void)
{
}

unsigned GOrgueStop::GetPipeCount()
{
	return m_Pipes.size();
}

void GOrgueStop::Abort()
{
	if (IsAuto())
		Set(false);
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->FastAbort();
}

void GOrgueStop::PreparePlayback()
{
	m_KeyState.resize(m_NumberOfAccessiblePipes);
	std::fill(m_KeyState.begin(), m_KeyState.end(), 0);

	if (IsAuto() && IsEngaged())
		m_Pipes[0]->Set(true);
}
