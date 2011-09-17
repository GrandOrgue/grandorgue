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

#include <wx/stream.h>
#include "GOrguePipe.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GOrgueManual.h"
#include "GOrgueWindchest.h"
#include "GOrgueReleaseAlignTable.h"
#include "GrandOrgueFile.h"
#include "GOSoundProviderSynthedTrem.h"
#include "GOSoundProviderWave.h"

extern GOrgueSound* g_sound;

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)
#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOrguePipe::~GOrguePipe()
{
	DELETE_AND_NULL(m_SoundProvider);
}

GOrguePipe::GOrguePipe
	(GrandOrgueFile* organfile
	,wxString filename
	,bool percussive
	,int samplerGroupID
	,int amplitude
	) :
	m_organfile(organfile),
	m_Sampler(NULL),
	pitch(0.0),
	instances(0),
	m_SamplerGroupID(samplerGroupID),
	m_filename(filename),
	m_percussive(percussive),
	m_amplitude(amplitude),
	m_ref(NULL),
	m_SoundProvider(NULL)
{
}

GOSoundProvider* GOrguePipe::GetSoundProvider()
{
	if (m_ref)
		return m_ref->GetSoundProvider();
	assert(m_SoundProvider);
	return m_SoundProvider;
}

void GOrguePipe::SetOn()
{
	if (instances > 0)
	{
		instances++;
	}
	else
	{
		m_Sampler = g_sound->GetEngine().StartSample(GetSoundProvider(), m_SamplerGroupID);
		if ((m_Sampler) && (instances == 0))
			instances++;
	}
}

void GOrguePipe::SetOff()
{
	if (instances > 0)
	{
		instances--;
		// If m_loop.data is null, the sample has no loop section which means
		// that it is a one-shot sample. We do not need to tell this sampler
		// to die.
		if ((!GetSoundProvider()->IsOneshot()) && (instances == 0))
		{
			g_sound->GetEngine().StopSample(GetSoundProvider(), m_Sampler);
			this->m_Sampler = 0;
		}
	}
}

void GOrguePipe::Set(bool on)
{
	if (m_ref)
	{
		m_ref->Set(on);
		return;
	}
	if (on)
		SetOn();
	else
		SetOff();
}

bool GOrguePipe::LoadCache(wxInputStream* cache)
{

	DELETE_AND_NULL(m_SoundProvider);

	if (m_filename.StartsWith(wxT("REF:")))
	{
		unsigned manual, stop, pipe;
		sscanf(m_filename.mb_str() + 4, "%d:%d:%d", &manual, &stop, &pipe);
		if ((manual < m_organfile->GetFirstManualIndex()) || (manual > m_organfile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > m_organfile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > m_organfile->GetManual(manual)->GetStop(stop-1)->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_filename;

		m_ref = m_organfile->GetManual(manual)->GetStop(stop-1)->GetPipe(pipe-1);

		return true;
	}

	int provider_type;
	cache->Read(&provider_type, sizeof(int));
	if (cache->LastRead() != sizeof(int))
		return false;

	switch (provider_type)
	{
	case 0:
		m_SoundProvider = new GOSoundProviderWave();
		break;
	case 1:
		m_SoundProvider = new GOSoundProviderSynthedTrem();
		break;
	default:
		throw (wxString)_("Invalid sound provider ID");
	}

	return m_SoundProvider->LoadCache(cache);

}

bool GOrguePipe::SaveCache(wxOutputStream* cache)
{

	if (m_ref)
		return true;

	int provider_type;
	if (dynamic_cast<GOSoundProviderWave*>(m_SoundProvider) != NULL)
		provider_type = 0;
	else if (dynamic_cast<GOSoundProviderSynthedTrem*>(m_SoundProvider) != NULL)
		provider_type = 1;
	else
		assert(0 && "Internal error");

	cache->Write(&provider_type, sizeof(int));
	if (cache->LastWrite() != (sizeof(int)))
		return false;

	return m_SoundProvider->SaveCache(cache);

}

void GOrguePipe::LoadData()
{

	DELETE_AND_NULL(m_SoundProvider);

	if (m_filename.StartsWith(wxT("REF:")))
	{
		unsigned manual, stop, pipe;
		sscanf(m_filename.mb_str() + 4, "%d:%d:%d", &manual, &stop, &pipe);
		if ((manual < m_organfile->GetFirstManualIndex()) || (manual > m_organfile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > m_organfile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > m_organfile->GetManual(manual)->GetStop(stop-1)->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_filename;

		m_ref = m_organfile->GetManual(manual)->GetStop(stop-1)->GetPipe(pipe-1);

		return;
	}

	GOSoundProviderWave *sp_wave = new GOSoundProviderWave();
	sp_wave->LoadFromFile(m_filename, m_amplitude);
	m_SoundProvider = sp_wave;

}

void GOrguePipe::CreateTremulant(int period, int startRate, int stopRate, int ampModDepth)
{

	DELETE_AND_NULL(m_SoundProvider);

	GOSoundProviderSynthedTrem *sp_trem = new GOSoundProviderSynthedTrem();
	sp_trem->Create(period, startRate, stopRate, ampModDepth);
	m_SoundProvider = sp_trem;

}

//FIXME: this function should not exist... it is here purely for legacy
//support in GOrgueSound::MIDIAllNotesOff()
void GOrguePipe::FastAbort()
{
	if (m_ref)
		m_ref->FastAbort();	
	if (instances > -1)
		instances = 0;
	m_Sampler = 0;

}

wxString GOrguePipe::GetFilename()
{
	return m_filename;
}
