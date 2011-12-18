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

#include "GOrgueStop.h"
#include "GOrguePipe.h"
#include "GrandOrgueFile.h"

GOrgueStop::GOrgueStop(GrandOrgueFile* organfile, unsigned manual_number, unsigned first_midi_note_number) :
	GOrgueDrawstop(organfile),
	m_Pipes(0),
	m_KeyState(0),
	m_ManualNumber(manual_number),
	m_FirstMidiNoteNumber(first_midi_note_number),
	m_Percussive(false),
	m_FirstAccessiblePipeLogicalPipeNumber(0),
	m_FirstAccessiblePipeLogicalKeyNumber(0),
	m_NumberOfAccessiblePipes(0),
	m_WindchestGroup(0),
	m_HarmonicNumber(8),
	m_PipeConfig(organfile, this)
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

GOrguePipeConfig& GOrgueStop::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOrgueStop::UpdateAmplitude()
{
        for(unsigned i = 0; i < m_Pipes.size(); i++)
                m_Pipes[i]->UpdateAmplitude();
}

void GOrgueStop::UpdateTuning()
{
        for(unsigned i = 0; i < m_Pipes.size(); i++)
                m_Pipes[i]->UpdateTuning();
}

void GOrgueStop::Load(IniFileConfig& cfg, wxString group)
{
	unsigned number_of_logical_pipes       = cfg.ReadInteger(group, wxT("NumberOfLogicalPipes"), 1, 192);
	m_PipeConfig.Load(cfg, group, wxEmptyString);
	m_FirstAccessiblePipeLogicalPipeNumber = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalPipeNumber"), 1, number_of_logical_pipes);
	m_FirstAccessiblePipeLogicalKeyNumber  = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalKeyNumber"), 1,  128);
	m_NumberOfAccessiblePipes              = cfg.ReadInteger(group, wxT("NumberOfAccessiblePipes"), 1, number_of_logical_pipes);
	m_WindchestGroup                       = cfg.ReadInteger(group, wxT("WindchestGroup"), 1, m_organfile->GetWinchestGroupCount());
	m_Percussive                           = cfg.ReadBoolean(group, wxT("Percussive"));
	m_HarmonicNumber                       = cfg.ReadInteger(group, wxT("HarmonicNumber"), 1, 1024, false, 8);
	m_PitchCorrection                      = cfg.ReadFloat(group, wxT("PitchCorrection"), -1200, 1200, false, 0);

        m_Pipes.clear();
        for (unsigned i = 0; i < number_of_logical_pipes; i++)
        {
                wxString buffer;
                buffer.Printf(wxT("Pipe%03u"), i + 1);
                m_Pipes.push_back
                        (new GOrguePipe
                                (m_organfile
                                ,this
                                ,m_Percussive
                                ,m_WindchestGroup
				,m_FirstMidiNoteNumber + i - m_FirstAccessiblePipeLogicalPipeNumber + m_FirstAccessiblePipeLogicalKeyNumber
				,m_HarmonicNumber
				,m_PitchCorrection
                                )
                        );
                m_Pipes[i]->Load(cfg, group, buffer);
        }
        m_KeyState.resize(m_NumberOfAccessiblePipes);
        std::fill(m_KeyState.begin(), m_KeyState.end(), 0);

        GOrgueDrawstop::Load(cfg, group);

}

void GOrgueStop::Save(IniFileConfig& cfg, bool prefix)
{
        GOrgueDrawstop::Save(cfg, prefix);
        for(unsigned i = 0; i < m_Pipes.size(); i++)
                m_Pipes[i]->Save(cfg, prefix);
        m_PipeConfig.Save(cfg, prefix);
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

void GOrgueStop::SetTemperament(const GOrgueTemperament& temperament)
{
	for(unsigned j = 0; j < m_Pipes.size(); j++)
		m_Pipes[j]->SetTemperament(temperament);
}
