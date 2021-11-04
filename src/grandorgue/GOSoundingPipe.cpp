/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSoundingPipe.h"

#include "GOAlloc.h"
#include "GOConfigReader.h"
#include "GOHash.h"
#include "go_limits.h"
#include "GOPath.h"
#include "GORank.h"
#include "GOSettings.h"
#include "GOTemperament.h"
#include "GOWindchest.h"
#include "GODefinitionFile.h"
#include <wx/intl.h>
#include <wx/log.h>

GOSoundingPipe::GOSoundingPipe(GODefinitionFile* organfile, GORank* rank, bool percussive, int sampler_group_id, unsigned midi_key_number, unsigned harmonic_number, float pitch_correction, float min_volume, float max_volume, bool retune) :
	GOPipe(organfile, rank, midi_key_number),
	m_Sampler(NULL),
	m_LastStop(0),
	m_Instances(0),
	m_Tremulant(false),
	m_AttackInfo(),
	m_ReleaseInfo(),
	m_Filename(),
	m_SamplerGroupID(sampler_group_id),
	m_AudioGroupID(0),
	m_Percussive(percussive),
	m_TemperamentOffset(0),
	m_HarmonicNumber(harmonic_number),
	m_LoopCrossfadeLength(0),
	m_ReleaseCrossfadeLength(0),
	m_PitchCorrection(pitch_correction),
	m_MinVolume(min_volume),
	m_MaxVolume(max_volume),
	m_SampleMidiKeyNumber(-1),
	m_RetunePipe(retune),
	m_SoundProvider(organfile->GetMemoryPool()),
	m_PipeConfig(&rank->GetPipeConfig(), organfile, this, &m_SoundProvider)
{
}

void GOSoundingPipe::LoadAttack(GOConfigReader& cfg, wxString group, wxString prefix)
{
	attack_load_info ainfo;
	ainfo.filename.Assign(cfg.ReadStringTrim(ODFSetting, group, prefix), m_organfile);
	ainfo.sample_group = cfg.ReadInteger(ODFSetting, group, prefix + wxT("IsTremulant"), -1, 1, false, -1);
	ainfo.load_release = cfg.ReadBoolean(ODFSetting, group, prefix + wxT("LoadRelease"), false, !m_Percussive);;
	ainfo.percussive = m_Percussive;
	ainfo.max_playback_time = cfg.ReadInteger(ODFSetting, group, prefix + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
	ainfo.cue_point = cfg.ReadInteger(ODFSetting, group, prefix + wxT("CuePoint"), -1, MAX_SAMPLE_LENGTH, false, -1);
	ainfo.min_attack_velocity = cfg.ReadInteger(ODFSetting, group, prefix + wxT("AttackVelocity"), 0, 127, false, 0);
	ainfo.max_released_time = cfg.ReadInteger(ODFSetting, group, prefix + wxT("MaxTimeSinceLastRelease"), -1, 100000, false, -1);
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

void GOSoundingPipe::Init(GOConfigReader& cfg, wxString group, wxString prefix, wxString filename)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = filename;
	m_PipeConfig.Init(cfg, group, prefix);
	m_SampleMidiKeyNumber = -1;
	m_LoopCrossfadeLength = 0;
	m_ReleaseCrossfadeLength = 0;
	UpdateAmplitude();
	m_organfile->GetWindchest(m_SamplerGroupID - 1)->AddPipe(this);

	attack_load_info ainfo;
	ainfo.filename.AssignResource(m_Filename, m_organfile);
	ainfo.sample_group = -1;
	ainfo.load_release = !m_Percussive;
	ainfo.percussive = m_Percussive;
	ainfo.max_playback_time = -1;
	ainfo.cue_point = -1;
	ainfo.min_attack_velocity = 0;
	ainfo.max_released_time = -1;
	ainfo.attack_start = 0;
	ainfo.release_end = -1;
	m_AttackInfo.push_back(ainfo);

	m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
	m_PipeConfig.SetName(wxString::Format(_("%d: %s"), m_MidiKeyNumber, m_Filename.c_str()));
}

void GOSoundingPipe::Load(GOConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	m_PipeConfig.Load(cfg, group, prefix);
	m_HarmonicNumber = cfg.ReadInteger(ODFSetting, group, prefix + wxT("HarmonicNumber"), 1, 1024, false, m_HarmonicNumber);
	m_PitchCorrection = cfg.ReadFloat(ODFSetting, group, prefix + wxT("PitchCorrection"), -1800, 1800, false, m_PitchCorrection);
	m_SamplerGroupID = cfg.ReadInteger(ODFSetting, group, prefix + wxT("WindchestGroup"), 1, m_organfile->GetWindchestGroupCount(), false, m_SamplerGroupID);
	m_Percussive = cfg.ReadBoolean(ODFSetting, group, prefix + wxT("Percussive"), false, m_Percussive);
	m_SampleMidiKeyNumber = cfg.ReadInteger(ODFSetting, group, prefix + wxT("MIDIKeyNumber"), -1, 127, false, -1);
	m_LoopCrossfadeLength = cfg.ReadInteger(ODFSetting, group, prefix + wxT("LoopCrossfadeLength"), 0, 120, false, 0);
	m_ReleaseCrossfadeLength = cfg.ReadInteger(ODFSetting, group, prefix + wxT("ReleaseCrossfadeLength"), 0, 200, false, 0);
	m_RetunePipe = cfg.ReadBoolean(ODFSetting, group, prefix + wxT("AcceptsRetuning"), false, m_RetunePipe);
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

		rinfo.filename.Assign(cfg.ReadStringTrim(ODFSetting, group, p), m_organfile);
		rinfo.sample_group = cfg.ReadInteger(ODFSetting, group, p + wxT("IsTremulant"), -1, 1, false, -1);
		rinfo.max_playback_time = cfg.ReadInteger(ODFSetting, group, p + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
		rinfo.cue_point = cfg.ReadInteger(ODFSetting, group, p + wxT("CuePoint"), -1, MAX_SAMPLE_LENGTH, false, -1);
		rinfo.release_end = cfg.ReadInteger(ODFSetting, group, p + wxT("ReleaseEnd"), -1, MAX_SAMPLE_LENGTH, false, -1);
		
		m_ReleaseInfo.push_back(rinfo);
	}

	m_MinVolume = cfg.ReadFloat(ODFSetting, group, wxT("MinVelocityVolume"), 0, 1000, false, m_MinVolume);
	m_MaxVolume = cfg.ReadFloat(ODFSetting, group, wxT("MaxVelocityVolume"), 0, 1000, false, m_MaxVolume);
	m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
	m_PipeConfig.SetName(wxString::Format(_("%d: %s"), m_MidiKeyNumber, m_Filename.c_str()));
}

void GOSoundingPipe::LoadData()
{
	try
	{
		m_SoundProvider.LoadFromFile(m_AttackInfo, m_ReleaseInfo, m_PipeConfig.GetEffectiveBitsPerSample(), m_PipeConfig.GetEffectiveChannels(), 
					     m_PipeConfig.GetEffectiveCompress(), (loop_load_type)m_PipeConfig.GetEffectiveLoopLoad(), m_PipeConfig.GetEffectiveAttackLoad(), m_PipeConfig.GetEffectiveReleaseLoad(),
					     m_SampleMidiKeyNumber, m_LoopCrossfadeLength, m_ReleaseCrossfadeLength);
		Validate();
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
		throw GOOutOfMemory();
	}
	catch(GOOutOfMemory e)
	{
		m_SoundProvider.ClearData();
		throw GOOutOfMemory();
	}
}

bool GOSoundingPipe::LoadCache(GOCache& cache)
{
	try
	{
		bool result = m_SoundProvider.LoadCache(cache);
		if (result)
			Validate();
		return result;
	}
	catch(std::bad_alloc& ba)
	{
		m_SoundProvider.ClearData();
		throw GOOutOfMemory();
	}
	catch(GOOutOfMemory e)
	{
		m_SoundProvider.ClearData();
		throw GOOutOfMemory();
	}
}

bool GOSoundingPipe::SaveCache(GOCacheWriter& cache)
{
	return m_SoundProvider.SaveCache(cache);
}

void GOSoundingPipe::UpdateHash(GOHash& hash)
{
	hash.Update(m_Filename);
	hash.Update(m_PipeConfig.GetEffectiveBitsPerSample());
	hash.Update(m_PipeConfig.GetEffectiveCompress());
	hash.Update(m_PipeConfig.GetEffectiveChannels());
	hash.Update(m_PipeConfig.GetEffectiveLoopLoad());
	hash.Update(m_PipeConfig.GetEffectiveAttackLoad());
	hash.Update(m_PipeConfig.GetEffectiveReleaseLoad());
	hash.Update(m_SampleMidiKeyNumber);
	hash.Update(m_LoopCrossfadeLength);
	hash.Update(m_ReleaseCrossfadeLength);

	hash.Update(m_AttackInfo.size());
	for(unsigned i = 0; i < m_AttackInfo.size(); i++)
	{
		m_AttackInfo[i].filename.Hash(hash);
		hash.Update(m_AttackInfo[i].sample_group);
		hash.Update(m_AttackInfo[i].max_playback_time);
		hash.Update(m_AttackInfo[i].load_release);
		hash.Update(m_AttackInfo[i].percussive);
		hash.Update(m_AttackInfo[i].cue_point);
		hash.Update(m_AttackInfo[i].loops.size());
		hash.Update(m_AttackInfo[i].attack_start);
		hash.Update(m_AttackInfo[i].release_end);
		for(unsigned j = 0; j < m_AttackInfo[i].loops.size(); j++)
		{
			hash.Update(m_AttackInfo[i].loops[j].loop_start);
			hash.Update(m_AttackInfo[i].loops[j].loop_end);
		}
	}

	hash.Update(m_ReleaseInfo.size());
	for(unsigned i = 0; i < m_ReleaseInfo.size(); i++)
	{
		m_ReleaseInfo[i].filename.Hash(hash);
		hash.Update(m_ReleaseInfo[i].sample_group);
		hash.Update(m_ReleaseInfo[i].max_playback_time);
		hash.Update(m_ReleaseInfo[i].cue_point);
		hash.Update(m_ReleaseInfo[i].release_end);
		}
}

void GOSoundingPipe::Initialize()
{
}

const wxString& GOSoundingPipe::GetLoadTitle()
{
	return m_Filename;
}

void GOSoundingPipe::Validate()
{
	if (!m_organfile->GetSettings().ODFCheck())
		return;

	if (!m_PipeConfig.GetEffectiveChannels())
		return;

	if (m_SoundProvider.checkForMissingAttack())
	{
		wxLogWarning(_("rank %s pipe %s: attack with MaxTimeSinceLastRelease=-1 missing"),
			     m_Rank->GetName().c_str(), GetLoadTitle().c_str());
	}

	if (m_SoundProvider.checkForMissingRelease())
	{
		wxLogWarning(_("rank %s pipe %s: default release is missing"),
			     m_Rank->GetName().c_str(), GetLoadTitle().c_str());
	}

	if (m_SoundProvider.checkMissingRelease())
	{
		wxLogWarning(_("rank %s pipe %s: no release defined"),
			     m_Rank->GetName().c_str(), GetLoadTitle().c_str());
	}

	if (m_SoundProvider.checkNotNecessaryRelease())
	{
		wxLogWarning(_("rank %s pipe %s: percussive sample with a release"),
			     m_Rank->GetName().c_str(), GetLoadTitle().c_str());
	}

	if (m_RetunePipe && m_SoundProvider.GetMidiKeyNumber() == 0 &&  m_SoundProvider.GetMidiPitchFract() == 0 && m_SampleMidiKeyNumber == -1)
	{
		wxLogWarning(_("rank %s pipe %s: no pitch information provided"),
			     m_Rank->GetName().c_str(), GetLoadTitle().c_str());
		return;
	}
	double offset;
	if (!m_RetunePipe)
		offset = 0;
	else
		offset= m_SoundProvider.GetMidiKeyNumber() + log(8.0 / m_HarmonicNumber) * (12.0 / log(2)) - (m_SoundProvider.GetMidiPitchFract() - m_PipeConfig.GetDefaultTuning() + m_PitchCorrection) / 100.0 - m_MidiKeyNumber;
	if (offset < -18 || offset > 18)
	{
		wxLogError(_("rank %s pipe %s: temperament would retune pipe by more than 1800 cent"),
			   m_Rank->GetName().c_str(), GetLoadTitle().c_str());
		return;
	}
	if (offset < -6 || offset > 6)
	{
		wxLogWarning(_("rank %s pipe %s: temperament would retune pipe by more than 600 cent"),
			   m_Rank->GetName().c_str(), GetLoadTitle().c_str());
		return;
	}
}

void GOSoundingPipe::SetTremulant(bool on)
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

GOSoundProvider* GOSoundingPipe::GetSoundProvider()
{
	return &m_SoundProvider;
}

void GOSoundingPipe::SetOn(unsigned velocity)
{
	m_Sampler = m_organfile->StartSample(GetSoundProvider(), m_SamplerGroupID, m_AudioGroupID, velocity, m_PipeConfig.GetEffectiveDelay(), m_LastStop);
	if (m_Sampler)
		m_Instances++;
	if (GetSoundProvider()->IsOneshot())
		m_Sampler = 0;
}

void GOSoundingPipe::SetOff()
{
	m_Instances--;
	if (m_Sampler)
	{
		m_LastStop = m_organfile->StopSample(GetSoundProvider(), m_Sampler);
		m_Sampler = 0;
	}
}

void GOSoundingPipe::Change(unsigned velocity, unsigned last_velocity)
{
	if (!m_Instances && velocity)
		SetOn(velocity);
	else if (m_Instances && !velocity)
		SetOff();
	else if (m_Sampler && last_velocity != velocity)
		m_organfile->UpdateVelocity(m_Sampler, velocity);
}

void GOSoundingPipe::UpdateAmplitude()
{
	m_SoundProvider.SetAmplitude(m_PipeConfig.GetEffectiveAmplitude(), m_PipeConfig.GetEffectiveGain());
}

void GOSoundingPipe::UpdateTuning()
{
	m_SoundProvider.SetTuning(m_PipeConfig.GetEffectiveTuning() + m_TemperamentOffset);
}

void GOSoundingPipe::UpdateAudioGroup()
{
	m_AudioGroupID = m_organfile->GetSettings().GetAudioGroupId(m_PipeConfig.GetEffectiveAudioGroup());
}

void GOSoundingPipe::SetTemperament(const GOTemperament& temperament)
{
	if (!m_RetunePipe)
		m_TemperamentOffset = 0;
	else
		m_TemperamentOffset = temperament.GetOffset(m_organfile->GetIgnorePitch(), m_MidiKeyNumber, m_SoundProvider.GetMidiKeyNumber(), m_SoundProvider.GetMidiPitchFract(), m_HarmonicNumber, m_PitchCorrection,
							    m_PipeConfig.GetDefaultTuning());
	UpdateTuning();
}

void GOSoundingPipe::PreparePlayback()
{
	UpdateAudioGroup();
	GOPipe::PreparePlayback();
}

void GOSoundingPipe::AbortPlayback()
{
	m_Instances = 0;
	m_Tremulant = false;
	m_Sampler = 0;
	m_LastStop = 0;
	m_SoundProvider.UseSampleGroup(0);
}
