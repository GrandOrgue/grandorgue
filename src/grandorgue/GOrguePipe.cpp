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

#include "GOrguePipe.h"
#include "GOrgueRank.h"
#include "GOrgueStop.h"
#include "GOrgueManual.h"
#include "GOrgueSettings.h"
#include "GOrgueTemperament.h"
#include "GrandOrgueFile.h"
#include "GOSoundProviderWave.h"
#include "IniFileConfig.h"

GOrguePipe::GOrguePipe
	(GrandOrgueFile* organfile
	,GOrgueRank* rank
	,bool percussive
	,int sampler_group_id
	,unsigned midi_key_number
	,unsigned harmonic_number
	,float pitch_correction
	) :
	m_OrganFile(organfile),
	m_Rank(rank),
	m_Sampler(NULL),
	m_Instances(0),
	m_SamplerGroupID(sampler_group_id),
	m_Filename(),
	m_Percussive(percussive),
	m_MidiKeyNumber(midi_key_number),
	m_TemperamentOffset(0),
	m_HarmonicNumber(harmonic_number),
	m_PitchCorrection(pitch_correction),
	m_Reference(NULL),
	m_SoundProvider(organfile->GetMemoryPool()),
	m_PipeConfig(organfile, this)
{
}

GOrguePipe::~GOrguePipe()
{
}

GOSoundProvider* GOrguePipe::GetSoundProvider()
{
	if (m_Reference)
		return m_Reference->GetSoundProvider();
	return &m_SoundProvider;
}

void GOrguePipe::SetOn()
{
	if (m_Instances > 0)
	{
		m_Instances++;
	}
	else
	{
		m_Sampler = m_OrganFile->StartSample(GetSoundProvider(), m_SamplerGroupID);
		if ((m_Sampler) && (m_Instances == 0))
			m_Instances++;
	}
}

void GOrguePipe::SetOff()
{
	if (m_Instances > 0)
	{
		m_Instances--;
		if ((!GetSoundProvider()->IsOneshot()) && (m_Instances == 0))
		{
			m_OrganFile->StopSample(GetSoundProvider(), m_Sampler);
			this->m_Sampler = 0;
		}
	}
}

void GOrguePipe::Set(bool on)
{
	if (m_Reference)
	{
		m_Reference->Set(on);
		return;
	}
	if (on)
		SetOn();
	else
		SetOff();
}

bool GOrguePipe::IsReference()
{
	return m_Reference != NULL;
}

GOrguePipeConfig& GOrguePipe::GetPipeConfig()
{
	return m_PipeConfig;
}

float GOrguePipe::GetEffectiveAmplitude()
{
	return m_PipeConfig.GetAmplitude() * m_Rank->GetPipeConfig().GetAmplitude() * m_OrganFile->GetPipeConfig().GetAmplitude();
}

void GOrguePipe::UpdateAmplitude()
{
	m_SoundProvider.SetAmplitude(GetEffectiveAmplitude());
}

float GOrguePipe::GetEffectiveTuning()
{
	return m_OrganFile->GetPipeConfig().GetTuning() + m_Rank->GetPipeConfig().GetTuning() + m_PipeConfig.GetTuning() + m_TemperamentOffset;
}

void GOrguePipe::UpdateTuning()
{
	m_SoundProvider.SetTuning(GetEffectiveTuning());
}

void GOrguePipe::Load(IniFileConfig& cfg, wxString group, wxString prefix)
{
	m_Filename = cfg.ReadString(group, prefix);
	m_PipeConfig.Load(cfg, group, prefix);
	m_HarmonicNumber = cfg.ReadInteger(group, prefix + wxT("HarmonicNumber"), 1, 1024, false, m_HarmonicNumber);
	m_PitchCorrection = cfg.ReadInteger(group, prefix + wxT("PitchCorrection"), -1200, 1200, false, m_PitchCorrection);
	UpdateAmplitude();
}

void GOrguePipe::Save(IniFileConfig& cfg, bool prefix)
{
	m_PipeConfig.Save(cfg, prefix);
}

bool GOrguePipe::LoadCache(GOrgueCache& cache)
{
	if (m_Filename.StartsWith(wxT("REF:")))
	{
		unsigned manual, stop, pipe;
		wxSscanf(m_Filename.Mid(4), wxT("%d:%d:%d"), &manual, &stop, &pipe);
		if ((manual < m_OrganFile->GetFirstManualIndex()) || (manual > m_OrganFile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > m_OrganFile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_Filename;
		m_Reference = m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetPipe(pipe-1);
		return true;
	}
	m_Reference = NULL;
	return m_SoundProvider.LoadCache(cache);
}

bool GOrguePipe::SaveCache(GOrgueCacheWriter& cache)
{
	if (m_Reference)
		return true;
	return m_SoundProvider.SaveCache(cache);
}

void GOrguePipe::LoadData()
{
	if (m_Filename.StartsWith(wxT("REF:")))
	{
		unsigned manual, stop, pipe;
		wxSscanf(m_Filename.Mid(4), wxT("%d:%d:%d"), &manual, &stop, &pipe);
		if ((manual < m_OrganFile->GetFirstManualIndex()) || (manual > m_OrganFile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > m_OrganFile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_Filename;
		m_Reference = m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetPipe(pipe-1);
		return;
	}
	m_Reference = NULL;
	m_SoundProvider.LoadFromFile
		(m_Filename
		,m_OrganFile->GetODFPath()
		,m_OrganFile->GetSettings().GetBitsPerSample()
		,m_OrganFile->GetSettings().GetLoadInStereo()
		,m_OrganFile->GetSettings().GetLosslessCompression()
		,(loop_load_type)m_OrganFile->GetSettings().GetLoopLoad()
		);
}

void GOrguePipe::FastAbort()
{
	if (m_Reference)
		m_Reference->FastAbort();
	m_Instances = 0;
	m_Sampler = 0;
}

wxString GOrguePipe::GetFilename()
{
	return m_Filename;
}

void GOrguePipe::SetTemperament(const GOrgueTemperament& temperament)
{
	m_TemperamentOffset = temperament.GetOffset(m_MidiKeyNumber, m_SoundProvider.GetMidiKeyNumber(), m_SoundProvider.GetMidiPitchFract(), m_HarmonicNumber, m_PitchCorrection,
						    m_OrganFile->GetPipeConfig().GetDefaultTuning() + m_Rank->GetPipeConfig().GetDefaultTuning() + m_PipeConfig.GetDefaultTuning());
	UpdateTuning();
}
