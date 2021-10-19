/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrguePipeConfigNode.h"

#include "GOrgueSampleStatistic.h"
#include "GOrgueStatisticCallback.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrguePipeConfigNode::GOrguePipeConfigNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback, GOrgueStatisticCallback* statistic) :
	m_organfile(organfile),
	m_parent(parent),
	m_PipeConfig(organfile, callback),
	m_StatisticCallback(statistic),
	m_Name()
{
	if (m_parent)
		m_parent->AddChild(this);
}

GOrguePipeConfigNode::~GOrguePipeConfigNode()
{
}

void GOrguePipeConfigNode::SetParent(GOrguePipeConfigNode* parent)
{
	m_parent = parent;
	if (m_parent)
		m_parent->AddChild(this);
}

const wxString& GOrguePipeConfigNode::GetName()
{
	return m_Name;
}

void GOrguePipeConfigNode::SetName(wxString name)
{
	m_Name = name;
}

void GOrguePipeConfigNode::Init(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterSaveableObject(this);
	m_PipeConfig.Init(cfg, group, prefix);
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

// Calculate the final AMPLITUDE value of a pipe, by summing child and rank values.
float GOrguePipeConfigNode::GetEffectiveAmplitude()
{
	if (m_parent)
		return m_PipeConfig.GetAmplitude() * m_parent->GetEffectiveAmplitude() / 100.0;
	else
		return m_PipeConfig.GetAmplitude() / 100.0;
}

// Calculate the final GAIN value of a pipe, by summing child and rank values.
float GOrguePipeConfigNode::GetEffectiveGain()
{
	if (m_parent)
		return m_PipeConfig.GetGain() + m_parent->GetEffectiveGain();
	else
		return m_PipeConfig.GetGain();
}

// Calculate the final TUNING value of a pipe, by summing child and rank values.
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

// Calculate the DELAY value of a child item (pipe), by summing child and rank DELAY values.
unsigned GOrguePipeConfigNode::GetEffectiveDelay()
{
	if (m_parent)
		return m_PipeConfig.GetDelay() + m_parent->GetEffectiveDelay();
	else
		return m_PipeConfig.GetDelay();
}

/// Calculate the final RELEASE TRUNCATION value for a pipe.
unsigned GOrguePipeConfigNode::GetEffectiveReleaseTruncationLength()
{
	if (m_parent)
		return m_PipeConfig.GetReleaseTruncationLength() + m_parent->GetEffectiveReleaseTruncationLength();
	else
		return m_PipeConfig.GetReleaseTruncationLength();
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
		return m_organfile->GetSettings().BitsPerSample();
}

bool GOrguePipeConfigNode::GetEffectiveCompress()
{
	if (m_PipeConfig.GetCompress() != -1)
		return m_PipeConfig.GetCompress() ? true : false;
	if (m_parent)
		return m_parent->GetEffectiveCompress();
	else
		return m_organfile->GetSettings().LosslessCompression();
}

unsigned GOrguePipeConfigNode::GetEffectiveLoopLoad()
{
	if (m_PipeConfig.GetLoopLoad() != -1)
		return m_PipeConfig.GetLoopLoad();
	if (m_parent)
		return m_parent->GetEffectiveLoopLoad();
	else
		return m_organfile->GetSettings().LoopLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveAttackLoad()
{
	if (m_PipeConfig.GetAttackLoad() != -1)
		return m_PipeConfig.GetAttackLoad();
	if (m_parent)
		return m_parent->GetEffectiveAttackLoad();
	else
		return m_organfile->GetSettings().AttackLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveReleaseLoad()
{
	if (m_PipeConfig.GetReleaseLoad() != -1)
		return m_PipeConfig.GetReleaseLoad();
	if (m_parent)
		return m_parent->GetEffectiveReleaseLoad();
	else
		return m_organfile->GetSettings().ReleaseLoad();
}

unsigned GOrguePipeConfigNode::GetEffectiveChannels()
{
	if (m_PipeConfig.GetChannels() != -1)
		return m_PipeConfig.GetChannels();
	if (m_parent)
		return m_parent->GetEffectiveChannels();
	else
		return m_organfile->GetSettings().LoadChannels();
}

GOrgueSampleStatistic GOrguePipeConfigNode::GetStatistic()
{
	if (m_StatisticCallback)
		return m_StatisticCallback->GetStatistic();
	return GOrgueSampleStatistic();
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
