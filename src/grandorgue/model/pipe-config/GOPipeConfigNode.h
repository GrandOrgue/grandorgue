/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIGNODE_H
#define GOPIPECONFIGNODE_H

#include "GOPipeConfig.h"
#include "GOSaveableObject.h"

class GOConfig;
class GOOrganModel;
class GOSampleStatistic;
class GOStatisticCallback;

class GOPipeConfigNode : private GOSaveableObject {
private:
  GOOrganModel &r_OrganModel;
  const GOConfig &m_config;
  GOPipeConfigNode *m_parent;
  GOPipeConfig m_PipeConfig;
  GOStatisticCallback *m_StatisticCallback;
  wxString m_Name;

  void Save(GOConfigWriter &cfg);

public:
  GOPipeConfigNode(
    GOPipeConfigNode *parent,
    GOOrganModel &organModel,
    GOPipeUpdateCallback *callback,
    GOStatisticCallback *statistic);
  virtual ~GOPipeConfigNode();

  GOPipeConfigNode *GetParent() const { return m_parent; }
  void SetParent(GOPipeConfigNode *parent);
  void Init(GOConfigReader &cfg, wxString group, wxString prefix);
  void Load(GOConfigReader &cfg, wxString group, wxString prefix);

  const wxString &GetName();
  void SetName(wxString name);

  GOPipeConfig &GetPipeConfig();

  void ModifyManualTuning(float diff);
  void ModifyAutoTuningCorrection(float diff);

  float GetEffectiveAmplitude();
  float GetEffectiveGain();
  float GetEffectivePitchTuning() const;
  float GetEffectivePitchCorrection() const;
  float GetEffectiveManualTuning() const;
  float GetEffectiveAutoTuningCorection() const;

  unsigned GetEffectiveDelay() const;
  wxString GetEffectiveAudioGroup() const;

  unsigned GetEffectiveBitsPerSample() const;
  bool GetEffectiveCompress() const;
  unsigned GetEffectiveLoopLoad() const;
  bool GetEffectiveAttackLoad() const;
  bool GetEffectiveReleaseLoad() const;
  unsigned GetEffectiveChannels() const;
  bool GetEffectiveIgnorePitch() const;
  unsigned GetEffectiveReleaseTail() const;

  virtual void AddChild(GOPipeConfigNode *node);
  virtual unsigned GetChildCount();
  virtual GOPipeConfigNode *GetChild(unsigned index);
  virtual GOSampleStatistic GetStatistic() const;
};

#endif
