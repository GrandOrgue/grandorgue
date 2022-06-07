/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeConfigTreeNode.h"

#include "GOSampleStatistic.h"

GOPipeConfigTreeNode::GOPipeConfigTreeNode(
  GOPipeConfigNode *parent,
  GODefinitionFile *organfile,
  GOPipeUpdateCallback *callback)
  : GOPipeConfigNode(parent, organfile, this, NULL),
    m_Childs(),
    m_Callback(callback) {}

void GOPipeConfigTreeNode::AddChild(GOPipeConfigNode *node) {
  m_Childs.push_back(node);
}

unsigned GOPipeConfigTreeNode::GetChildCount() { return m_Childs.size(); }

GOPipeConfigNode *GOPipeConfigTreeNode::GetChild(unsigned index) {
  return m_Childs[index];
}

void GOPipeConfigTreeNode::UpdateAmplitude() {
  for (unsigned i = 0; i < m_Childs.size(); i++)
    m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAmplitude();
  if (m_Callback)
    m_Callback->UpdateAmplitude();
}

void GOPipeConfigTreeNode::UpdateTuning() {
  for (unsigned i = 0; i < m_Childs.size(); i++)
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

void GOPipeConfigTreeNode::UpdateAudioGroup() {
  for (unsigned i = 0; i < m_Childs.size(); i++)
    m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAudioGroup();
  if (m_Callback)
    m_Callback->UpdateAudioGroup();
}

GOSampleStatistic GOPipeConfigTreeNode::GetStatistic() {
  GOSampleStatistic stat = GOPipeConfigNode::GetStatistic();
  for (unsigned i = 0; i < m_Childs.size(); i++)
    stat.Cumulate(m_Childs[i]->GetStatistic());
  return stat;
}
