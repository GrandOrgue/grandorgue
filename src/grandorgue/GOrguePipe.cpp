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

#include "GOrguePipe.h"

#include "GOrgueConfigReader.h"
#include "GOrgueManual.h"
#include "GOrgueRank.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>
#include <wx/tokenzr.h>

GOrguePipe::GOrguePipe (GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number) :
	m_Velocity(0),
	m_Velocities(1),
	m_organfile(organfile),
	m_Rank(rank),
	m_Filename(),
	m_MidiKeyNumber(midi_key_number),
	m_Reference(NULL),
	m_ReferenceID(0),
	m_PipeConfig(&rank->GetPipeConfig(), organfile, this)
{
}

GOrguePipe::~GOrguePipe()
{
}

unsigned GOrguePipe::RegisterReference(GOrguePipe* pipe)
{
	unsigned id = m_Velocities.size();
	m_Velocities.resize(id + 1);
	return id;
}

void GOrguePipe::Abort()
{
}

void GOrguePipe::PreparePlayback()
{
	m_Velocity = 0;
	for(unsigned i = 0; i < m_Velocities.size(); i++)
		m_Velocities[i] = 0;
}

void GOrguePipe::SetTemperament(const GOrgueTemperament& temperament)
{
	
}

void GOrguePipe::Change(unsigned velocity, unsigned last_velocity)
{
	if (m_Reference)
	{
		m_Reference->Set(velocity, m_ReferenceID);
		return;
	}
}

void GOrguePipe::Set(unsigned velocity, unsigned referenceID)
{
	if (m_Velocities[referenceID] <= velocity && velocity <= m_Velocity)
	{
		m_Velocities[referenceID] = velocity;
		return;
	}
	unsigned last_velocity = m_Velocity;
	if (velocity >= m_Velocity)
	{
		m_Velocities[referenceID] = velocity;
		m_Velocity = velocity;
	}
	else
	{
		m_Velocities[referenceID] = velocity;
		m_Velocity = m_Velocities[0];
		for(unsigned i = 1; i < m_Velocities.size(); i++)
			if (m_Velocity < m_Velocities[i])
				m_Velocity = m_Velocities[i];
	}
	Change(m_Velocity, last_velocity);
}

void GOrguePipe::SetTremulant(bool on)
{
}

bool GOrguePipe::IsReference()
{
	return m_Reference != NULL;
}

GOrguePipeConfigNode& GOrguePipe::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOrguePipe::UpdateAmplitude()
{
}

void GOrguePipe::UpdateTuning()
{
}

void GOrguePipe::UpdateAudioGroup()
{
}

void GOrguePipe::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	if (m_Filename.StartsWith(wxT("REF:")))
		return;
}

bool GOrguePipe::InitializeReference()
{
	if (m_Filename.StartsWith(wxT("REF:")))
	{
		unsigned long manual, stop, pipe;
		wxArrayString strs = wxStringTokenize(m_Filename.Mid(4), wxT(":"), wxTOKEN_RET_EMPTY_ALL);
		if (strs.GetCount() != 3 ||
		    !strs[0].ToULong(&manual) ||
		    !strs[1].ToULong(&stop) ||
		    !strs[2].ToULong(&pipe))
			throw (wxString)_("Invalid reference ") + m_Filename;
		if ((manual < m_organfile->GetFirstManualIndex()) || (manual >= m_organfile->GetODFManualCount()) ||
			(stop <= 0) || (stop > m_organfile->GetManual(manual)->GetStopCount()) ||
		    (pipe <= 0) || (pipe > m_organfile->GetManual(manual)->GetStop(stop-1)->GetRank(0)->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_Filename;
		m_Reference = m_organfile->GetManual(manual)->GetStop(stop-1)->GetRank(0)->GetPipe(pipe-1);
		m_ReferenceID = m_Reference->RegisterReference(this);
		return true;
	}
	return false;
}

bool GOrguePipe::LoadCache(GOrgueCache& cache)
{
	InitializeReference();
	return true;
}

bool GOrguePipe::SaveCache(GOrgueCacheWriter& cache)
{
	return true;
}

void GOrguePipe::Initialize()
{
	InitializeReference();
}

void GOrguePipe::UpdateHash(SHA_CTX& ctx)
{
}

void GOrguePipe::LoadData()
{
	if (InitializeReference())
		return;
	m_Reference = NULL;
}

wxString GOrguePipe::GetFilename()
{
	return m_Filename;
}

const wxString& GOrguePipe::GetLoadTitle()
{
	return m_Filename;
}

unsigned GOrguePipe::GetMidiKeyNumber()
{
	return m_MidiKeyNumber;
}
