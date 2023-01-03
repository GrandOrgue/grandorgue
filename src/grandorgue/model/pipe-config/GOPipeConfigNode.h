/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIGNODE_H
#define GOPIPECONFIGNODE_H

#include "GOPipeConfig.h"
#include "GOSaveableObject.h"

class GOOrganModel;
class GOSampleStatistic;
class GOStatisticCallback;

class GOPipeConfigNode : private GOSaveableObject {
private:
  GOOrganModel *m_OrganModel;
  GOPipeConfigNode *m_parent;
  GOPipeConfig m_PipeConfig;
  GOStatisticCallback *m_StatisticCallback;
  wxString m_Name;

  void Save(GOConfigWriter &cfg);

public:
  GOPipeConfigNode(
    GOPipeConfigNode *parent,
    GOOrganModel *organModel,
    GOPipeUpdateCallback *callback,
    GOStatisticCallback *statistic);
  virtual ~GOPipeConfigNode();

  void SetParent(GOPipeConfigNode *parent);
  void Init(GOConfigReader &cfg, wxString group, wxString prefix);
  void Load(GOConfigReader &cfg, wxString group, wxString prefix);

  const wxString &GetName();
  void SetName(wxString name);

  GOPipeConfig &GetPipeConfig();

  void ModifyTuning(float diff);

  float GetEffectiveAmplitude();
  float GetEffectiveGain();
  float GetEffectiveTuning();
  float GetDefaultTuning();

  unsigned GetEffectiveDelay();
  wxString GetEffectiveAudioGroup();

  unsigned GetEffectiveBitsPerSample();
  bool GetEffectiveCompress();
  unsigned GetEffectiveLoopLoad();
  unsigned GetEffectiveAttackLoad();
  unsigned GetEffectiveReleaseLoad();
  unsigned GetEffectiveChannels();
  bool GetEffectiveIgnorePitch();

  virtual void AddChild(GOPipeConfigNode *node);
  virtual unsigned GetChildCount();
  virtual GOPipeConfigNode *GetChild(unsigned index);
  virtual GOSampleStatistic GetStatistic();
};

#endif
