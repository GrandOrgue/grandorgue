/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPECONFIGTREENODE_H
#define GORGUEPIPECONFIGTREENODE_H

#include "GOrguePipeConfigNode.h"
#include <vector>

class GOrguePipeConfigTreeNode : public GOrguePipeConfigNode, private GOrguePipeUpdateCallback
{
private:
	std::vector<GOrguePipeConfigNode*> m_Childs;
	GOrguePipeUpdateCallback* m_Callback;

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateAudioGroup();

public:
	GOrguePipeConfigTreeNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback);

	void AddChild(GOrguePipeConfigNode* node);
	unsigned GetChildCount();
	GOrguePipeConfigNode* GetChild(unsigned index);
	GOrgueSampleStatistic GetStatistic();
};

#endif
