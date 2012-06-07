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
#include "GOrguePipe.h"
#include "GOrgueRank.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

GOrgueRank::GOrgueRank(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Name(),
	m_Group(),
	m_Pipes(),
	m_FirstMidiNoteNumber(0),
	m_Percussive(false),
	m_WindchestGroup(0),
	m_HarmonicNumber(8),
	m_PitchCorrection(0),
	m_PipeConfig(organfile, this)
{
}

GOrgueRank::~GOrgueRank()
{
}

void GOrgueRank::Load(GOrgueConfigReader& cfg, wxString group, int first_midi_note_number)
{
	m_FirstMidiNoteNumber = cfg.ReadInteger(ODFSetting, group, wxT("FirstMidiNoteNumber"), 0, 256, false, first_midi_note_number);
	m_Group = group;
	m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"), 64, true);

	unsigned number_of_logical_pipes       = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLogicalPipes"), 1, 192);
	m_PipeConfig.Load(cfg, group, wxEmptyString);
	m_WindchestGroup                       = cfg.ReadInteger(ODFSetting, group, wxT("WindchestGroup"), 1, m_organfile->GetWindchestGroupCount());
	m_Percussive                           = cfg.ReadBoolean(ODFSetting, group, wxT("Percussive"));
	m_HarmonicNumber                       = cfg.ReadInteger(ODFSetting, group, wxT("HarmonicNumber"), 1, 1024, false, 8);
	m_PitchCorrection                      = cfg.ReadFloat(ODFSetting, group, wxT("PitchCorrection"), -1200, 1200, false, 0);

	m_organfile->GetWindchest(m_WindchestGroup - 1)->AddRank(this);

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
			    ,m_FirstMidiNoteNumber + i
			    ,m_HarmonicNumber
			    ,m_PitchCorrection
			    )
			   );
               m_Pipes[i]->Load(cfg, group, buffer);
       }
}

void GOrgueRank::Save(GOrgueConfigWriter& cfg)
{
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->Save(cfg);
	m_PipeConfig.Save(cfg);
}

void GOrgueRank::SetKey(int note, bool on)
{
	if (note < 0 || note >= (int)m_Pipes.size())
		return;

	m_Pipes[note]->Set(on);
}

GOrguePipe* GOrgueRank::GetPipe(unsigned index)
{
	return m_Pipes[index];
}

unsigned GOrgueRank::GetPipeCount()
{
	return m_Pipes.size();
}

const wxString& GOrgueRank::GetName()
{
	return m_Name;
}

GOrguePipeConfig& GOrgueRank::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOrgueRank::UpdateAmplitude()
{
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->UpdateAmplitude();
}

void GOrgueRank::UpdateTuning()
{
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->UpdateTuning();
}

void GOrgueRank::UpdateAudioGroup()
{
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->UpdateAudioGroup();
}

void GOrgueRank::SetTemperament(const GOrgueTemperament& temperament)
{
	for(unsigned j = 0; j < m_Pipes.size(); j++)
		m_Pipes[j]->SetTemperament(temperament);
}

void GOrgueRank::Abort()
{
	for(unsigned i = 0; i < m_Pipes.size(); i++)
		m_Pipes[i]->FastAbort();
}

void GOrgueRank::PreparePlayback()
{
	UpdateAudioGroup();
}

