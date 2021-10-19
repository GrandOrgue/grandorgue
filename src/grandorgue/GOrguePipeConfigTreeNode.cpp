/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrguePipeConfigTreeNode.h"

#include "GOrgueSampleStatistic.h"

GOrguePipeConfigTreeNode::GOrguePipeConfigTreeNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback) :
	GOrguePipeConfigNode(parent, organfile, this, NULL),
	m_Childs(),
	m_Callback(callback)
{
}

void GOrguePipeConfigTreeNode::AddChild(GOrguePipeConfigNode* node)
{
	m_Childs.push_back(node);
}

unsigned GOrguePipeConfigTreeNode::GetChildCount()
{
	return m_Childs.size();
}

GOrguePipeConfigNode* GOrguePipeConfigTreeNode::GetChild(unsigned index)
{
	return m_Childs[index];
}
// NOTE: Update callback items must be listed in GOrguePipeUpdateCallback.h in the "core" folder.
void GOrguePipeConfigTreeNode::UpdateAmplitude()
{
	for(unsigned i = 0; i < m_Childs.size(); i++)
		m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAmplitude();
	if (m_Callback)
		m_Callback->UpdateAmplitude();
}

void GOrguePipeConfigTreeNode::UpdateTuning()
{
	for(unsigned i = 0; i < m_Childs.size(); i++)
		m_Childs[i]->GetPipeConfig().GetCallback()->UpdateTuning();
	if (m_Callback)
		m_Callback->UpdateTuning();
}

void GOrguePipeConfigTreeNode::UpdateReleaseTruncationLength()
{
	for(unsigned i = 0; i < m_Childs.size(); i++)
		m_Childs[i]->GetPipeConfig().GetCallback()->UpdateReleaseTruncationLength();
	if (m_Callback)
		m_Callback->UpdateReleaseTruncationLength();
}

void GOrguePipeConfigTreeNode::UpdateAudioGroup()
{
	for(unsigned i = 0; i < m_Childs.size(); i++)
		m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAudioGroup();
	if (m_Callback)
		m_Callback->UpdateAudioGroup();
}

GOrgueSampleStatistic GOrguePipeConfigTreeNode::GetStatistic()
{
	GOrgueSampleStatistic stat = GOrguePipeConfigNode::GetStatistic();
	for(unsigned i = 0; i < m_Childs.size(); i++)
		stat.Cumulate(m_Childs[i]->GetStatistic());
	return stat;
}
