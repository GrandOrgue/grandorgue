/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
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
  void UpdateToneBalance() override;

public:
  GOPipeConfigTreeNode(
    GOPipeConfigNode *parent,
    GOOrganModel *organModel,
    GOPipeUpdateCallback *callback);

  /**
   * Detaches the children that are still alive, so that their destructors do
   * not touch this destroyed node
   */
  ~GOPipeConfigTreeNode();

  void AddChild(GOPipeConfigNode *node) override { m_Childs.push_back(node); }

  /** Removes the child from the list of children, if it is there. */
  void RemoveChild(GOPipeConfigNode *node) override;
  unsigned GetChildCount() const override { return m_Childs.size(); }
  GOPipeConfigNode *GetChild(unsigned index) const override {
    return m_Childs[index];
  }
  GOSampleStatistic GetStatistic() const override;
};

#endif
