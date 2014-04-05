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

#include "GOrguePipeConfigTreeNode.h"


GOrguePipeConfigTreeNode::GOrguePipeConfigTreeNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback) :
	GOrguePipeConfigNode(parent, organfile, this),
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

void GOrguePipeConfigTreeNode::UpdateAudioGroup()
{
	for(unsigned i = 0; i < m_Childs.size(); i++)
		m_Childs[i]->GetPipeConfig().GetCallback()->UpdateAudioGroup();
	if (m_Callback)
		m_Callback->UpdateAudioGroup();
}
