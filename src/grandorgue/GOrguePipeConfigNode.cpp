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

#include "GOrguePipeConfigNode.h"

#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrguePipeConfigNode::GOrguePipeConfigNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback) :
	m_organfile(organfile),
	m_parent(parent),
	m_PipeConfig(organfile, callback),
	m_Name()
{
	if (m_parent)
		m_parent->AddChild(this);
}

GOrguePipeConfigNode::~GOrguePipeConfigNode()
{
}

const wxString& GOrguePipeConfigNode::GetName()
{
	return m_Name;
}

void GOrguePipeConfigNode::SetName(wxString name)
{
	m_Name = name;
}

void GOrguePipeConfigNode::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterSaveableObject(this);
	m_PipeConfig.Load(cfg, group, prefix);
}

void GOrguePipeConfigNode::Save(GOrgueConfigWriter& cfg)
{
	m_PipeConfig.Save(cfg);
}

GOrguePipeConfig& GOrguePipeConfigNode::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOrguePipeConfigNode::ModifyTuning(float diff)
{
	m_PipeConfig.SetTuning(m_PipeConfig.GetTuning() + diff);
}

float GOrguePipeConfigNode::GetEffectiveAmplitude()
{
	if (m_parent)
		return m_PipeConfig.GetAmplitude() * m_parent->GetEffectiveAmplitude();
	else
		return m_PipeConfig.GetAmplitude();
}

float GOrguePipeConfigNode::GetEffectiveGain()
{
	if (m_parent)
		return m_PipeConfig.GetGain() + m_parent->GetEffectiveGain();
	else
		return m_PipeConfig.GetGain();
}

float GOrguePipeConfigNode::GetEffectiveTuning()
{
	if (m_parent)
		return m_PipeConfig.GetTuning() + m_parent->GetEffectiveTuning();
	else
		return m_PipeConfig.GetTuning();
}

float GOrguePipeConfigNode::GetDefaultTuning()
{
	if (m_parent)
		return m_PipeConfig.GetDefaultTuning() + m_parent->GetDefaultTuning();
	else
		return m_PipeConfig.GetDefaultTuning();
}

unsigned GOrguePipeConfigNode::GetEffectiveDelay()
{
	if (m_parent)
		return m_PipeConfig.GetDelay() + m_parent->GetEffectiveDelay();
	else
		return m_PipeConfig.GetDelay();
}

wxString GOrguePipeConfigNode::GetEffectiveAudioGroup()
{
	if (m_PipeConfig.GetAudioGroup() != wxEmptyString)
		return m_PipeConfig.GetAudioGroup();
	if (m_parent)
		return m_parent->GetEffectiveAudioGroup();
	else
		return wxEmptyString;
}

unsigned GOrguePipeConfigNode::GetEffectiveBitsPerSample()
{
	if (m_PipeConfig.GetBitsPerSample() != -1)
		return m_PipeConfig.GetBitsPerSample();
	if (m_parent)
		return m_parent->GetEffectiveBitsPerSample();
	else
		return m_organfile->GetSettings().GetBitsPerSample();
}

bool GOrguePipeConfigNode::GetEffectiveCompress()
{
	if (m_PipeConfig.GetCompress() != -1)
		return m_PipeConfig.GetCompress() ? true : false;
	if (m_parent)
		return m_parent->GetEffectiveCompress();
	else
		return m_organfile->GetSettings().GetLosslessCompression();
}

unsigned GOrguePipeConfigNode::GetEffectiveLoopLoad()
{
	if (m_PipeConfig.GetLoopLoad() != -1)
		return m_PipeConfig.GetLoopLoad();
	if (m_parent)
		return m_parent->GetEffectiveLoopLoad();
	else
		return m_organfile->GetSettings().GetLoopLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveAttackLoad()
{
	if (m_PipeConfig.GetAttackLoad() != -1)
		return m_PipeConfig.GetAttackLoad();
	if (m_parent)
		return m_parent->GetEffectiveAttackLoad();
	else
		return m_organfile->GetSettings().GetAttackLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveReleaseLoad()
{
	if (m_PipeConfig.GetReleaseLoad() != -1)
		return m_PipeConfig.GetReleaseLoad();
	if (m_parent)
		return m_parent->GetEffectiveReleaseLoad();
	else
		return m_organfile->GetSettings().GetReleaseLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveChannels()
{
	if (m_PipeConfig.GetChannels() != -1)
		return m_PipeConfig.GetChannels();
	if (m_parent)
		return m_parent->GetEffectiveChannels();
	else
		return m_organfile->GetSettings().GetLoadChannels();
}

unsigned GOrguePipeConfigNode::GetChildCount()
{
	return 0;
}

GOrguePipeConfigNode* GOrguePipeConfigNode::GetChild(unsigned index)
{
	return NULL;
}

void GOrguePipeConfigNode::AddChild(GOrguePipeConfigNode* node)
{
}
