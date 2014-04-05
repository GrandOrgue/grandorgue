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

#include "GOSoundProviderWave.h"
#include "GOrgueConfigReader.h"
#include "GOrgueLimits.h"
#include "GOrgueManual.h"
#include "GOrgueRank.h"
#include "GOrgueSettings.h"
#include "GOrgueStop.h"
#include "GOrgueTemperament.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>
#include <wx/tokenzr.h>

GOrguePipe::GOrguePipe (GrandOrgueFile* organfile, GOrgueRank* rank, bool percussive, int sampler_group_id, unsigned midi_key_number, unsigned harmonic_number, float pitch_correction, float min_volume, float max_volume) :
	m_organfile(organfile),
	m_Rank(rank),
	m_Sampler(NULL),
	m_Instances(0),
	m_Tremulant(false),
	m_AttackInfo(),
	m_ReleaseInfo(),
	m_SamplerGroupID(sampler_group_id),
	m_AudioGroupID(0),
	m_Filename(),
	m_Percussive(percussive),
	m_MidiKeyNumber(midi_key_number),
	m_TemperamentOffset(0),
	m_HarmonicNumber(harmonic_number),
	m_PitchCorrection(pitch_correction),
	m_MinVolume(min_volume),
	m_MaxVolume(max_volume),
	m_SampleMidiKeyNumber(-1),
	m_Reference(NULL),
	m_ReferenceID(0),
	m_SoundProvider(organfile->GetMemoryPool()),
	m_PipeConfig(&rank->GetPipeConfig(), organfile, this),
	m_Velocity(0),
	m_Velocities(1)
{
}

GOrguePipe::~GOrguePipe()
{
}

GOSoundProvider* GOrguePipe::GetSoundProvider()
{
	if (m_Reference)
		return NULL;
	return &m_SoundProvider;
}

void GOrguePipe::SetOn(unsigned velocity)
{
	m_Sampler = m_organfile->StartSample(GetSoundProvider(), m_SamplerGroupID, m_AudioGroupID, velocity, m_PipeConfig.GetEffectiveDelay());
	if (m_Sampler)
		m_Instances++;
	if (GetSoundProvider()->IsOneshot())
		m_Sampler = 0;
}

void GOrguePipe::SetOff()
{
	m_Instances--;
	if (m_Sampler)
	{
		m_organfile->StopSample(GetSoundProvider(), m_Sampler);
		this->m_Sampler = 0;
	}
}

void GOrguePipe::Change(unsigned velocity, unsigned last_velocity)
{
	if (m_Reference)
	{
		m_Reference->Set(velocity, m_ReferenceID);
		return;
	}

	if (!m_Instances && velocity)
		SetOn(velocity);
	else if (m_Instances && !velocity)
		SetOff();
	else if (m_Sampler && last_velocity != velocity)
		m_organfile->UpdateVelocity(m_Sampler, velocity);
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
	if (on)
	{
		if (!m_Tremulant)
		{
			m_Tremulant = true;
			m_SoundProvider.UseSampleGroup(1);
			if (m_Sampler)
				m_organfile->SwitchSample(GetSoundProvider(), m_Sampler);
		}
	}
	else
	{
		if (m_Tremulant)
		{
			m_Tremulant = false;
			m_SoundProvider.UseSampleGroup(0);
			if (m_Sampler)
				m_organfile->SwitchSample(GetSoundProvider(), m_Sampler);
		}
	}
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
	m_SoundProvider.SetAmplitude(m_PipeConfig.GetEffectiveAmplitude(), m_PipeConfig.GetEffectiveGain());
}

void GOrguePipe::UpdateTuning()
{
	m_SoundProvider.SetTuning(m_PipeConfig.GetEffectiveTuning() + m_TemperamentOffset);
}

void GOrguePipe::UpdateAudioGroup()
{
	m_AudioGroupID = m_organfile->GetSettings().GetAudioGroupId(m_PipeConfig.GetEffectiveAudioGroup());
}

void GOrguePipe::LoadAttack(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	attack_load_info ainfo;
	ainfo.filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	ainfo.sample_group = cfg.ReadInteger(ODFSetting, group, prefix + wxT("IsTremulant"), -1, 1, false, -1);
	ainfo.load_release = cfg.ReadBoolean(ODFSetting, group, prefix + wxT("LoadRelease"), false, !m_Percussive);;
	ainfo.percussive = m_Percussive;
	ainfo.max_playback_time = cfg.ReadInteger(ODFSetting, group, prefix + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
	ainfo.cue_point = cfg.ReadInteger(ODFSetting, group, prefix + wxT("CuePoint"), -1, MAX_SAMPLE_LENGTH, false, -1);
	ainfo.min_attack_velocity = cfg.ReadInteger(ODFSetting, group, prefix + wxT("AttackVelocity"), 0, 127, false, 0);
	ainfo.attack_start = cfg.ReadInteger(ODFSetting, group, prefix + wxT("AttackStart"), 0, MAX_SAMPLE_LENGTH, false, 0);
	ainfo.release_end = cfg.ReadInteger(ODFSetting, group, prefix + wxT("ReleaseEnd"), -1, MAX_SAMPLE_LENGTH, false, -1);

	unsigned loop_cnt = cfg.ReadInteger(ODFSetting, group, prefix + wxT("LoopCount"), 0, 100, false, 0);
	for(unsigned j = 0; j < loop_cnt; j++)
	{
		loop_load_info linfo;
		linfo.loop_start = cfg.ReadInteger(ODFSetting, group, prefix + wxString::Format(wxT("Loop%03dStart"), j + 1), 0, MAX_SAMPLE_LENGTH, false, 0);
		linfo.loop_end = cfg.ReadInteger(ODFSetting, group, prefix + wxString::Format(wxT("Loop%03dEnd"), j + 1), linfo.loop_start + 1, MAX_SAMPLE_LENGTH, true);
		ainfo.loops.push_back(linfo);
	}

	m_AttackInfo.push_back(ainfo);
}

void GOrguePipe::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	if (m_Filename.StartsWith(wxT("REF:")))
		return;
	m_PipeConfig.Load(cfg, group, prefix);
	m_HarmonicNumber = cfg.ReadInteger(ODFSetting, group, prefix + wxT("HarmonicNumber"), 1, 1024, false, m_HarmonicNumber);
	m_PitchCorrection = cfg.ReadFloat(ODFSetting, group, prefix + wxT("PitchCorrection"), -1200, 1200, false, m_PitchCorrection);
	m_SamplerGroupID = cfg.ReadInteger(ODFSetting, group, prefix + wxT("WindchestGroup"), 1, m_organfile->GetWindchestGroupCount(), false, m_SamplerGroupID);
	m_Percussive = cfg.ReadBoolean(ODFSetting, group, prefix + wxT("Percussive"), false, m_Percussive);
	m_SampleMidiKeyNumber = cfg.ReadInteger(ODFSetting, group, prefix + wxT("MIDIKeyNumber"), -1, 127, false, -1);
	UpdateAmplitude();
	m_organfile->GetWindchest(m_SamplerGroupID - 1)->AddPipe(this);

	LoadAttack(cfg, group, prefix);

	unsigned attack_count = cfg.ReadInteger(ODFSetting, group, prefix + wxT("AttackCount"), 0, 100, false, 0);
	for(unsigned i = 0; i < attack_count; i++)
		LoadAttack(cfg, group, prefix + wxString::Format(wxT("Attack%03d"), i + 1));

	unsigned release_count = cfg.ReadInteger(ODFSetting, group, prefix + wxT("ReleaseCount"), 0, 100, false, 0);
	for(unsigned i = 0; i < release_count; i++)
	{
		release_load_info rinfo;
		wxString p = prefix + wxString::Format(wxT("Release%03d"), i + 1);

		rinfo.filename = cfg.ReadStringTrim(ODFSetting, group, p);
		rinfo.sample_group = cfg.ReadInteger(ODFSetting, group, p + wxT("IsTremulant"), -1, 1, false, -1);
		rinfo.max_playback_time = cfg.ReadInteger(ODFSetting, group, p + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
		rinfo.cue_point = cfg.ReadInteger(ODFSetting, group, p + wxT("CuePoint"), -1, MAX_SAMPLE_LENGTH, false, -1);
		rinfo.release_end = cfg.ReadInteger(ODFSetting, group, p + wxT("ReleaseEnd"), -1, MAX_SAMPLE_LENGTH, false, -1);
		
		m_ReleaseInfo.push_back(rinfo);
	}

	m_MinVolume = cfg.ReadFloat(ODFSetting, group, wxT("MinVelocityVolume"), 0, 1000, false, m_MinVolume);
	m_MaxVolume = cfg.ReadFloat(ODFSetting, group, wxT("MaxVelocityVolume"), 0, 1000, false, m_MaxVolume);
	m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
	m_PipeConfig.SetName(wxString::Format(_("%d: %s"), GetMidiKeyNumber(), GetFilename().c_str()));
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

unsigned GOrguePipe::RegisterReference(GOrguePipe* pipe)
{
	unsigned id = m_Velocities.size();
	m_Velocities.resize(id + 1);
	return id;
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
	catch(std::bad_alloc& ba)
	{
		m_SoundProvider.ClearData();
		throw GOrgueOutOfMemory();
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
	SHA1_Update(&ctx, (const wxChar*)filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
	value = m_PipeConfig.GetEffectiveBitsPerSample();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_PipeConfig.GetEffectiveCompress();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_PipeConfig.GetEffectiveChannels();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_PipeConfig.GetEffectiveLoopLoad();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_PipeConfig.GetEffectiveAttackLoad();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_PipeConfig.GetEffectiveReleaseLoad();
	SHA1_Update(&ctx, &value, sizeof(value));
	value = m_SampleMidiKeyNumber;
	SHA1_Update(&ctx, &value, sizeof(value));

	value = m_AttackInfo.size();
	SHA1_Update(&ctx, &value, sizeof(value));
	for(unsigned i = 0; i < m_AttackInfo.size(); i++)
	{
		filename = m_AttackInfo[i].filename;
		SHA1_Update(&ctx, (const wxChar*)filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
		value = m_AttackInfo[i].sample_group;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].max_playback_time;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].load_release;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].percussive;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].cue_point;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].loops.size();
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].attack_start;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_AttackInfo[i].release_end;
		SHA1_Update(&ctx, &value, sizeof(value));
		for(unsigned j = 0; j < m_AttackInfo[i].loops.size(); j++)
		{
			value = m_AttackInfo[i].loops[j].loop_start;
			SHA1_Update(&ctx, &value, sizeof(value));
			value = m_AttackInfo[i].loops[j].loop_end;
			SHA1_Update(&ctx, &value, sizeof(value));
		}
	}

	value = m_ReleaseInfo.size();
	SHA1_Update(&ctx, &value, sizeof(value));
	for(unsigned i = 0; i < m_ReleaseInfo.size(); i++)
	{
		filename = m_ReleaseInfo[i].filename;
		SHA1_Update(&ctx, (const wxChar*)filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
		value = m_ReleaseInfo[i].sample_group;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_ReleaseInfo[i].max_playback_time;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_ReleaseInfo[i].cue_point;
		SHA1_Update(&ctx, &value, sizeof(value));
		value = m_ReleaseInfo[i].release_end;
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
		m_SoundProvider.LoadFromFile(m_AttackInfo, m_ReleaseInfo, m_organfile->GetODFPath(), m_PipeConfig.GetEffectiveBitsPerSample(), m_PipeConfig.GetEffectiveChannels(), 
					     m_PipeConfig.GetEffectiveCompress(), (loop_load_type)m_PipeConfig.GetEffectiveLoopLoad(), m_PipeConfig.GetEffectiveAttackLoad(), m_PipeConfig.GetEffectiveReleaseLoad(),
					     m_SampleMidiKeyNumber);
	}
	catch(wxString str)
	{
		m_SoundProvider.ClearData();
		throw wxString::Format(_("Error while loading samples for rank %s pipe %s: %s"),
				       m_Rank->GetName().c_str(), GetLoadTitle().c_str(), str.c_str());
	}
	catch(std::bad_alloc& ba)
	{
		m_SoundProvider.ClearData();
		throw GOrgueOutOfMemory();
	}
	catch(GOrgueOutOfMemory e)
	{
		m_SoundProvider.ClearData();
		throw GOrgueOutOfMemory();
	}
}

void GOrguePipe::Abort()
{
	if (m_Reference)
		m_Reference->Abort();
	m_Instances = 0;
	m_Tremulant = false;
	m_Sampler = 0;
	m_SoundProvider.UseSampleGroup(0);
}

void GOrguePipe::PreparePlayback()
{
	UpdateAudioGroup();
	if (m_Reference)
		m_Reference->PreparePlayback();
	m_Velocity = 0;
	for(unsigned i = 0; i < m_Velocities.size(); i++)
		m_Velocities[i] = 0;
}

wxString GOrguePipe::GetFilename()
{
	return m_Filename;
}

const wxString& GOrguePipe::GetLoadTitle()
{
	return m_Filename;
}

void GOrguePipe::SetTemperament(const GOrgueTemperament& temperament)
{
	
	m_TemperamentOffset = temperament.GetOffset(m_organfile->GetIgnorePitch(), m_MidiKeyNumber, m_SoundProvider.GetMidiKeyNumber(), m_SoundProvider.GetMidiPitchFract(), m_HarmonicNumber, m_PitchCorrection,
						    m_PipeConfig.GetDefaultTuning());
	UpdateTuning();
}

unsigned GOrguePipe::GetMidiKeyNumber()
{
	return m_MidiKeyNumber;
}
