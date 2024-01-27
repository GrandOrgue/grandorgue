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

  void Save(GOConfigWriter &cfg) override { m_PipeConfig.Save(cfg); }

  float GetEffectiveFloatSum(
    float (GOPipeConfig::*getFloat)() const,
    float (GOPipeConfigNode::*getParentFloat)() const) const;

public:
  GOPipeConfigNode(
    GOPipeConfigNode *parent,
    GOOrganModel &organModel,
    GOPipeUpdateCallback *callback,
    GOStatisticCallback *statistic);

  GOPipeConfigNode *GetParent() const { return m_parent; }
  void SetParent(GOPipeConfigNode *parent);
  void Init(GOConfigReader &cfg, const wxString &group, const wxString &prefix);
  void Load(GOConfigReader &cfg, const wxString &group, const wxString &prefix);

  const wxString &GetName() const { return m_Name; }
  void SetName(const wxString &name) { m_Name = name; }

  GOPipeConfig &GetPipeConfig() { return m_PipeConfig; }

  wxString GetEffectiveAudioGroup() const;

  float GetEffectiveAmplitude() const;

  float GetEffectiveGain() const {
    return GetEffectiveFloatSum(
      &GOPipeConfig::GetGain, &GOPipeConfigNode::GetEffectiveGain);
  }

  float GetEffectivePitchTuning() const {
    return GetEffectiveFloatSum(
      &GOPipeConfig::GetPitchTuning,
      &GOPipeConfigNode::GetEffectivePitchTuning);
  }

  float GetEffectivePitchCorrection() const {
    return GetEffectiveFloatSum(
      &GOPipeConfig::GetPitchCorrection,
      &GOPipeConfigNode::GetEffectivePitchCorrection);
  }

  float GetEffectiveManualTuning() const {
    return GetEffectiveFloatSum(
      &GOPipeConfig::GetManualTuning,
      &GOPipeConfigNode::GetEffectiveManualTuning);
  }

  float GetEffectiveAutoTuningCorection() const {
    return GetEffectiveFloatSum(
      &GOPipeConfig::GetAutoTuningCorrection,
      &GOPipeConfigNode::GetEffectiveAutoTuningCorection);
  }

  unsigned GetEffectiveDelay() const;
  unsigned GetEffectiveReleaseTail() const;
  unsigned GetEffectiveBitsPerSample() const;
  unsigned GetEffectiveChannels() const;
  unsigned GetEffectiveLoopLoad() const;
  bool GetEffectiveCompress() const;
  bool GetEffectiveAttackLoad() const;
  bool GetEffectiveReleaseLoad() const;
  bool GetEffectiveIgnorePitch() const;

  virtual void AddChild(GOPipeConfigNode *node) {}
  virtual unsigned GetChildCount() const { return 0; }
  virtual GOPipeConfigNode *GetChild(unsigned index) const { return nullptr; }
  virtual GOSampleStatistic GetStatistic() const;

  void ModifyManualTuning(float diff);
  void ModifyAutoTuningCorrection(float diff);
};

#endif
