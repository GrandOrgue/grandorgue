/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOPipeConfigNode.h"

#include "GOSampleStatistic.h"
#include "GOStatisticCallback.h"
#include "config/GOConfig.h"
#include "GODefinitionFile.h"

GOPipeConfigNode::GOPipeConfigNode(GOPipeConfigNode* parent, GODefinitionFile* organfile, GOPipeUpdateCallback* callback, GOStatisticCallback* statistic) :
	m_organfile(organfile),
	m_parent(parent),
	m_PipeConfig(organfile, callback),
	m_StatisticCallback(statistic),
	m_Name()
{
	if (m_parent)
		m_parent->AddChild(this);
}

GOPipeConfigNode::~GOPipeConfigNode()
{
}

void GOPipeConfigNode::SetParent(GOPipeConfigNode* parent)
{
	m_parent = parent;
	if (m_parent)
		m_parent->AddChild(this);
}

const wxString& GOPipeConfigNode::GetName()
{
	return m_Name;
}

void GOPipeConfigNode::SetName(wxString name)
{
	m_Name = name;
}

void GOPipeConfigNode::Init(GOConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterSaveableObject(this);
	m_PipeConfig.Init(cfg, group, prefix);
}

void GOPipeConfigNode::Load(GOConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterSaveableObject(this);
	m_PipeConfig.Load(cfg, group, prefix);
}

void GOPipeConfigNode::Save(GOConfigWriter& cfg)
{
	m_PipeConfig.Save(cfg);
}

GOPipeConfig& GOPipeConfigNode::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOPipeConfigNode::ModifyTuning(float diff)
{
	m_PipeConfig.SetTuning(m_PipeConfig.GetTuning() + diff);
}

float GOPipeConfigNode::GetEffectiveAmplitude()
{
	if (m_parent)
		return m_PipeConfig.GetAmplitude() * m_parent->GetEffectiveAmplitude() / 100.0;
	else
		return m_PipeConfig.GetAmplitude() / 100.0;
}

float GOPipeConfigNode::GetEffectiveGain()
{
	if (m_parent)
		return m_PipeConfig.GetGain() + m_parent->GetEffectiveGain();
	else
		return m_PipeConfig.GetGain();
}

float GOPipeConfigNode::GetEffectiveTuning()
{
	if (m_parent)
		return m_PipeConfig.GetTuning() + m_parent->GetEffectiveTuning();
	else
		return m_PipeConfig.GetTuning();
}

float GOPipeConfigNode::GetDefaultTuning()
{
	if (m_parent)
		return m_PipeConfig.GetDefaultTuning() + m_parent->GetDefaultTuning();
	else
		return m_PipeConfig.GetDefaultTuning();
}

unsigned GOPipeConfigNode::GetEffectiveDelay()
{
	if (m_parent)
		return m_PipeConfig.GetDelay() + m_parent->GetEffectiveDelay();
	else
		return m_PipeConfig.GetDelay();
}

wxString GOPipeConfigNode::GetEffectiveAudioGroup()
{
	if (m_PipeConfig.GetAudioGroup() != wxEmptyString)
		return m_PipeConfig.GetAudioGroup();
	if (m_parent)
		return m_parent->GetEffectiveAudioGroup();
	else
		return wxEmptyString;
}

unsigned GOPipeConfigNode::GetEffectiveBitsPerSample()
{
	if (m_PipeConfig.GetBitsPerSample() != -1)
		return m_PipeConfig.GetBitsPerSample();
	if (m_parent)
		return m_parent->GetEffectiveBitsPerSample();
	else
		return m_organfile->GetSettings().BitsPerSample();
}

bool GOPipeConfigNode::GetEffectiveCompress()
{
	if (m_PipeConfig.GetCompress() != -1)
		return m_PipeConfig.GetCompress() ? true : false;
	if (m_parent)
		return m_parent->GetEffectiveCompress();
	else
		return m_organfile->GetSettings().LosslessCompression();
}

unsigned GOPipeConfigNode::GetEffectiveLoopLoad()
{
	if (m_PipeConfig.GetLoopLoad() != -1)
		return m_PipeConfig.GetLoopLoad();
	if (m_parent)
		return m_parent->GetEffectiveLoopLoad();
	else
		return m_organfile->GetSettings().LoopLoad();
}

unsigned GOPipeConfigNode::GetEffectiveAttackLoad()
{
	if (m_PipeConfig.GetAttackLoad() != -1)
		return m_PipeConfig.GetAttackLoad();
	if (m_parent)
		return m_parent->GetEffectiveAttackLoad();
	else
		return m_organfile->GetSettings().AttackLoad();
}

unsigned GOPipeConfigNode::GetEffectiveReleaseLoad()
{
	if (m_PipeConfig.GetReleaseLoad() != -1)
		return m_PipeConfig.GetReleaseLoad();
	if (m_parent)
		return m_parent->GetEffectiveReleaseLoad();
	else
		return m_organfile->GetSettings().ReleaseLoad();
}

unsigned GOPipeConfigNode::GetEffectiveChannels()
{
	if (m_PipeConfig.GetChannels() != -1)
		return m_PipeConfig.GetChannels();
	if (m_parent)
		return m_parent->GetEffectiveChannels();
	else
		return m_organfile->GetSettings().LoadChannels();
}

GOSampleStatistic GOPipeConfigNode::GetStatistic()
{
	if (m_StatisticCallback)
		return m_StatisticCallback->GetStatistic();
	return GOSampleStatistic();
}

unsigned GOPipeConfigNode::GetChildCount()
{
	return 0;
}

GOPipeConfigNode* GOPipeConfigNode::GetChild(unsigned index)
{
	return NULL;
}

void GOPipeConfigNode::AddChild(GOPipeConfigNode* node)
{
}
