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

#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

const struct IniFileEnumEntry GOrgueTremulant::m_tremulant_types[]={
	{ wxT("Synth"), GOSynthTrem },
};

GOrgueTremulant::GOrgueTremulant(GrandOrgueFile* organfile) :
	GOrgueDrawstop(organfile),
	m_TremulantType(GOSynthTrem),
	m_Period(0),
	m_StartRate(0),
	m_StopRate(0),
	m_AmpModDepth(0),
	m_TremProvider(organfile->GetMemoryPool()),
	m_PlaybackHandle(0),
	m_SamplerGroupID(0)
{
}

GOrgueTremulant::~GOrgueTremulant()
{
}

void GOrgueTremulant::Initialize()
{
}

void GOrgueTremulant::LoadData()
{
	InitSoundProvider();
}

bool GOrgueTremulant::LoadCache(GOrgueCache& cache)
{
	InitSoundProvider();
	return true;
}

bool GOrgueTremulant::SaveCache(GOrgueCacheWriter& cache)
{
	return true;
}

void GOrgueTremulant::UpdateHash(SHA_CTX& ctx)
{
}

wxString GOrgueTremulant::GetLoadTitle()
{
	return m_Name;
}

void GOrgueTremulant::Load(IniFileConfig& cfg, wxString group, int sampler_group_id)
{
	m_TremulantType = (GOrgueTremulantType)cfg.ReadEnum(group, wxT("TremulantType"), m_tremulant_types, sizeof(m_tremulant_types) / sizeof(m_tremulant_types[0]), false, GOSynthTrem);
	if (m_TremulantType == GOSynthTrem)
	{
		m_Period            = cfg.ReadLong(group, wxT("Period"), 32, 441000);
		m_StartRate         = cfg.ReadInteger(group, wxT("StartRate"), 1, 100);
		m_StopRate          = cfg.ReadInteger(group, wxT("StopRate"), 1, 100);
		m_AmpModDepth       = cfg.ReadInteger(group, wxT("AmpModDepth"), 1, 100);
		m_SamplerGroupID    = sampler_group_id;
		m_PlaybackHandle    = 0;
	}
	GOrgueDrawstop::Load(cfg, group);
}

void GOrgueTremulant::InitSoundProvider()
{
	if (m_TremulantType == GOSynthTrem)
	{
		m_TremProvider.Create(m_Period, m_StartRate, m_StopRate, m_AmpModDepth);
		assert(!m_TremProvider.IsOneshot());
	}
}

void GOrgueTremulant::Save(IniFileConfig& cfg, bool prefix)
{
	GOrgueDrawstop::Save(cfg, prefix);
}

void GOrgueTremulant::Set(bool on)
{
	if (IsEngaged() != on && m_TremulantType == GOSynthTrem)
	{
		if (on)
		{
			assert(m_SamplerGroupID < 0);
			m_PlaybackHandle = m_organfile->StartSample(&m_TremProvider, m_SamplerGroupID);
			on = (m_PlaybackHandle != NULL);
		}
		else
		{
			assert(m_PlaybackHandle);
			m_organfile->StopSample(&m_TremProvider, m_PlaybackHandle);
			m_PlaybackHandle = NULL;
		}
	}
	GOrgueDrawstop::Set(on);
}

void GOrgueTremulant::Abort()
{
	m_PlaybackHandle = NULL;
}

void GOrgueTremulant::PreparePlayback()
{
	if (IsEngaged() && m_TremulantType == GOSynthTrem)
	{
		assert(m_SamplerGroupID < 0);
		m_PlaybackHandle = m_organfile->StartSample(&m_TremProvider, m_SamplerGroupID);
	}
	if (m_TremulantType == GOWavTrem)
	{
		m_organfile->ControlChanged((GOrgueButton*)this);
	}
}

GOrgueTremulantType GOrgueTremulant::GetTremulantType()
{
	return m_TremulantType;
}
