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
 * MA 02111-1307, USA.
 */

#include "GOrguePipe.h"
#include "GOrgueRank.h"
#include "GOrgueStop.h"
#include "GOrgueManual.h"
#include "GOrgueSettings.h"
#include "GOrgueTemperament.h"
#include "GOrgueWindchest.h"
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
	m_Tremulant(false),
	m_AttackInfo(),
	m_ReleaseInfo(),
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
		if (GetSoundProvider()->IsOneshot())
			m_Sampler = 0;
	}
}

void GOrguePipe::SetOff()
{
	if (m_Instances > 0)
	{
		m_Instances--;
		if ((m_Sampler) && (m_Instances == 0))
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

void GOrguePipe::SetTremulant(bool on)
{
	if (on)
	{
		if (!m_Tremulant)
		{
			m_Tremulant = true;
			m_SoundProvider.UseSampleGroup(1);
			if (m_Sampler)
				m_OrganFile->SwitchSample(GetSoundProvider(), m_Sampler);
		}
	}
	else
	{
		if (m_Tremulant)
		{
			m_Tremulant = false;
			m_SoundProvider.UseSampleGroup(0);
			if (m_Sampler)
				m_OrganFile->SwitchSample(GetSoundProvider(), m_Sampler);
		}
	}
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

unsigned GOrguePipe::GetEffectiveBitsPerSample()
{
	if (m_PipeConfig.GetBitsPerSample() != -1)
		return m_PipeConfig.GetBitsPerSample();
	if (m_Rank->GetPipeConfig().GetBitsPerSample() != -1)
		return m_Rank->GetPipeConfig().GetBitsPerSample();
	if (m_OrganFile->GetPipeConfig().GetBitsPerSample() != -1)
		return m_OrganFile->GetPipeConfig().GetBitsPerSample();
	return m_OrganFile->GetSettings().GetBitsPerSample();
}

bool GOrguePipe::GetEffectiveCompress()
{
	if (m_PipeConfig.GetCompress() != -1)
		return m_PipeConfig.GetCompress() ? true : false;
	if (m_Rank->GetPipeConfig().GetCompress() != -1)
		return m_Rank->GetPipeConfig().GetCompress() ? true : false;
	if (m_OrganFile->GetPipeConfig().GetCompress() != -1)
		return m_OrganFile->GetPipeConfig().GetCompress() ? true : false;
	return m_OrganFile->GetSettings().GetLosslessCompression();
}

unsigned GOrguePipe::GetEffectiveLoopLoad()
{
	if (m_PipeConfig.GetLoopLoad() != -1)
		return m_PipeConfig.GetLoopLoad();
	if (m_Rank->GetPipeConfig().GetLoopLoad() != -1)
		return m_Rank->GetPipeConfig().GetLoopLoad();
	if (m_OrganFile->GetPipeConfig().GetLoopLoad() != -1)
		return m_OrganFile->GetPipeConfig().GetLoopLoad();
	return m_OrganFile->GetSettings().GetLoopLoad();
}

unsigned GOrguePipe::GetEffectiveChannels()
{
	if (m_PipeConfig.GetChannels() != -1)
		return m_PipeConfig.GetChannels();
	if (m_Rank->GetPipeConfig().GetChannels() != -1)
		return m_Rank->GetPipeConfig().GetChannels();
	if (m_OrganFile->GetPipeConfig().GetChannels() != -1)
		return m_OrganFile->GetPipeConfig().GetChannels();
	return m_OrganFile->GetSettings().GetLoadInStereo() ? 2 : 1;
}

void GOrguePipe::Load(IniFileConfig& cfg, wxString group, wxString prefix)
{
	m_Filename = cfg.ReadString(group, prefix);
	m_PipeConfig.Load(cfg, group, prefix);
	m_HarmonicNumber = cfg.ReadInteger(group, prefix + wxT("HarmonicNumber"), 1, 1024, false, m_HarmonicNumber);
	m_PitchCorrection = cfg.ReadInteger(group, prefix + wxT("PitchCorrection"), -1200, 1200, false, m_PitchCorrection);
	m_SamplerGroupID = cfg.ReadInteger(group, prefix + wxT("WindchestGroup"), 1, m_OrganFile->GetWinchestGroupCount(), false, m_SamplerGroupID);
	UpdateAmplitude();
	m_OrganFile->GetWindchest(m_SamplerGroupID - 1)->AddPipe(this);

	attack_load_info ainfo;
	ainfo.filename = m_Filename;
	ainfo.sample_group = -1;
	ainfo.load_release = true;
	ainfo.percussive = m_Percussive;
	ainfo.max_playback_time = -1;
	m_AttackInfo.push_back(ainfo);
}

void GOrguePipe::Save(IniFileConfig& cfg, bool prefix)
{
	m_PipeConfig.Save(cfg, prefix);
}

bool GOrguePipe::InitializeReference()
{
	if (m_Filename.StartsWith(wxT("REF:")))
	{
		unsigned manual, stop, pipe;
		wxSscanf(m_Filename.Mid(4), wxT("%d:%d:%d"), &manual, &stop, &pipe);
		if ((manual < m_OrganFile->GetFirstManualIndex()) || (manual > m_OrganFile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > m_OrganFile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetRank()->GetPipeCount()))
			throw (wxString)_("Invalid reference ") + m_Filename;
		m_Reference = m_OrganFile->GetManual(manual)->GetStop(stop-1)->GetRank()->GetPipe(pipe-1);
		return true;
	}
	return false;
}

bool GOrguePipe::LoadCache(GOrgueCache& cache)
{
	if (InitializeReference())
		return true;
	m_Reference = NULL;
	try
	{
		return m_SoundProvider.LoadCache(cache);
	}
	catch(GOrgueOutOfMemory e)
	{
		m_SoundProvider.ClearData();
		throw GOrgueOutOfMemory();
	}
}

bool GOrguePipe::SaveCache(GOrgueCacheWriter& cache)
{
	if (m_Reference)
		return true;
	return m_SoundProvider.SaveCache(cache);
}

void GOrguePipe::Initialize()
{
	InitializeReference();
}

void GOrguePipe::UpdateHash(SHA_CTX& ctx)
{
	unsigned value;
	wxString filename = GetFilename();
	SHA1_Update(&ctx, filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
	value = GetEffectiveBitsPerSample();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = GetEffectiveCompress();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = GetEffectiveChannels();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = GetEffectiveLoopLoad();
	SHA1_Update(&ctx, &value, sizeof(value));

	value = m_AttackInfo.size();
	SHA1_Update(&ctx, &value, sizeof(value));
	for(unsigned i = 0; i < m_AttackInfo.size(); i++)
	{
		filename = m_AttackInfo[i].filename;
		SHA1_Update(&ctx, filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
		value = m_AttackInfo[i].sample_group;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].max_playback_time;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].load_release;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].percussive;
		SHA1_Update(&ctx, &value, sizeof(value));
	}

	value = m_ReleaseInfo.size();
	SHA1_Update(&ctx, &value, sizeof(value));
	for(unsigned i = 0; i < m_ReleaseInfo.size(); i++)
	{
		filename = m_ReleaseInfo[i].filename;
		SHA1_Update(&ctx, filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
		value = m_ReleaseInfo[i].sample_group;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_ReleaseInfo[i].max_playback_time;
		SHA1_Update(&ctx, &value, sizeof(value));
	}
}

void GOrguePipe::LoadData()
{
	if (InitializeReference())
		return;
	m_Reference = NULL;
	try
	{
		m_SoundProvider.LoadFromFile(m_AttackInfo, m_ReleaseInfo, m_OrganFile->GetODFPath(), GetEffectiveBitsPerSample(), GetEffectiveChannels(), 
					     GetEffectiveCompress(), (loop_load_type)GetEffectiveLoopLoad());
	}
	catch(GOrgueOutOfMemory e)
	{
		m_SoundProvider.ClearData();
		throw GOrgueOutOfMemory();
	}
}

void GOrguePipe::FastAbort()
{
	if (m_Reference)
		m_Reference->FastAbort();
	m_Instances = 0;
	m_Tremulant = false;
	m_Sampler = 0;
	m_SoundProvider.UseSampleGroup(0);
}

wxString GOrguePipe::GetFilename()
{
	return m_Filename;
}

wxString GOrguePipe::GetLoadTitle()
{
	return m_Filename;
}

void GOrguePipe::SetTemperament(const GOrgueTemperament& temperament)
{
	
	m_TemperamentOffset = temperament.GetOffset(m_OrganFile->GetIgnorePitch(), m_MidiKeyNumber, m_SoundProvider.GetMidiKeyNumber(), m_SoundProvider.GetMidiPitchFract(), m_HarmonicNumber, m_PitchCorrection,
						    m_OrganFile->GetPipeConfig().GetDefaultTuning() + m_Rank->GetPipeConfig().GetDefaultTuning() + m_PipeConfig.GetDefaultTuning());
	UpdateTuning();
}
