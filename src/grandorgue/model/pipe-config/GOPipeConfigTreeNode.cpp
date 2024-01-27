/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeConfigTreeNode.h"

#include "GOSampleStatistic.h"

GOPipeConfigTreeNode::GOPipeConfigTreeNode(
  GOPipeConfigNode *parent,
  GOOrganModel *organModel,
  GOPipeUpdateCallback *callback)
  : GOPipeConfigNode(parent, *organModel, this, NULL),
    m_Childs(),
    m_Callback(callback) {}

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

void GOPipeConfigTreeNode::UpdateAudioGroup() {
  for (unsigned i = 0; i < m_Childs.size(); i++)
    m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAudioGroup();
  if (m_Callback)
    m_Callback->UpdateAudioGroup();
}

void GOPipeConfigTreeNode::UpdateReleaseTail() {
  for (auto child : m_Childs)
    child->GetPipeConfig().GetCallback()->UpdateReleaseTail();
  if (m_Callback)
    m_Callback->UpdateReleaseTail();
}

GOSampleStatistic GOPipeConfigTreeNode::GetStatistic() const {
  GOSampleStatistic stat = GOPipeConfigNode::GetStatistic();
  for (unsigned i = 0; i < m_Childs.size(); i++)
    stat.Cumulate(m_Childs[i]->GetStatistic());
  return stat;
}
