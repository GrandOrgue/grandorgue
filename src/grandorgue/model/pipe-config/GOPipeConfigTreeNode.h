/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIGTREENODE_H
#define GOPIPECONFIGTREENODE_H

#include <vector>

#include "GOPipeConfigNode.h"
#include "GOPipeUpdateCallback.h"

class GOPipeConfigTreeNode : public GOPipeConfigNode,
                             private GOPipeUpdateCallback {
private:
  std::vector<GOPipeConfigNode *> m_Childs;
  GOPipeUpdateCallback *m_Callback;

  void UpdateAmplitude() override;
  void UpdateTuning() override;
  void UpdateAudioGroup() override;
  void UpdateReleaseTail() override;

public:
  GOPipeConfigTreeNode(
    GOPipeConfigNode *parent,
    GOOrganModel *organModel,
    GOPipeUpdateCallback *callback);

  void AddChild(GOPipeConfigNode *node);
  unsigned GetChildCount();
  GOPipeConfigNode *GetChild(unsigned index);
  GOSampleStatistic GetStatistic() const;
};

#endif
