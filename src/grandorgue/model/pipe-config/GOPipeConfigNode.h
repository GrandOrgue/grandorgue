/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIGNODE_H
#define GOPIPECONFIGNODE_H

#include <config/GOConfig.h>

#include "GOPipeConfig.h"
#include "GOSaveableObject.h"

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

  template <typename T>
  T GetEffectiveSum(
    T (GOPipeConfig::*getThisValue)() const,
    T (GOPipeConfigNode::*getParentValue)() const) const {
    T value = (m_PipeConfig.*getThisValue)();

    if (m_parent)
      value += (m_parent->*getParentValue)();
    return value;
  }

  uint8_t GetEffectiveUint8(
    int8_t (GOPipeConfig::*getThisValue)() const,
    uint8_t (GOPipeConfigNode::*getParentValue)() const,
    const GOSettingUnsigned GOConfig::*globalValue) const;

  template <typename T>
  bool GetEffectiveBool(
    GOBool3 (GOPipeConfig::*getThisValue)() const,
    bool (GOPipeConfigNode::*getParentValue)() const,
    const T GOConfig::*globalValue) const {
    return to_bool((m_PipeConfig.*getThisValue)(), [=]() {
      return m_parent ? (m_parent->*getParentValue)()
                      : globalValue && (bool)(m_config.*globalValue)();
    });
  }

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
    return GetEffectiveSum(
      &GOPipeConfig::GetGain, &GOPipeConfigNode::GetEffectiveGain);
  }

  float GetEffectivePitchTuning() const {
    return GetEffectiveSum(
      &GOPipeConfig::GetPitchTuning,
      &GOPipeConfigNode::GetEffectivePitchTuning);
  }

  float GetEffectivePitchCorrection() const {
    return GetEffectiveSum(
      &GOPipeConfig::GetPitchCorrection,
      &GOPipeConfigNode::GetEffectivePitchCorrection);
  }

  float GetEffectiveManualTuning() const {
    return GetEffectiveSum(
      &GOPipeConfig::GetManualTuning,
      &GOPipeConfigNode::GetEffectiveManualTuning);
  }

  float GetEffectiveAutoTuningCorection() const {
    return GetEffectiveSum(
      &GOPipeConfig::GetAutoTuningCorrection,
      &GOPipeConfigNode::GetEffectiveAutoTuningCorection);
  }

  uint16_t GetEffectiveDelay() const {
    return GetEffectiveSum(
      &GOPipeConfig::GetDelay, &GOPipeConfigNode::GetEffectiveDelay);
  }

  uint16_t GetEffectiveReleaseTail() const;

  uint8_t GetEffectiveBitsPerSample() const {
    return GetEffectiveUint8(
      &GOPipeConfig::GetBitsPerSample,
      &GOPipeConfigNode::GetEffectiveBitsPerSample,
      (const GOSettingUnsigned GOConfig::*)&GOConfig::BitsPerSample);
  }

  uint8_t GetEffectiveChannels() const {
    return GetEffectiveUint8(
      &GOPipeConfig::GetChannels,
      &GOPipeConfigNode::GetEffectiveChannels,
      &GOConfig::LoadChannels);
  }

  uint8_t GetEffectiveLoopLoad() const {
    return GetEffectiveUint8(
      &GOPipeConfig::GetLoopLoad,
      &GOPipeConfigNode::GetEffectiveLoopLoad,
      &GOConfig::LoopLoad);
  }

  bool GetEffectivePercussive() const {
    return GetEffectiveBool(
      &GOPipeConfig::GetPercussive,
      &GOPipeConfigNode::GetEffectivePercussive,
      (const GOSettingUnsigned GOConfig::*)nullptr);
  }

  bool IsEffectiveIndependentRelease() const {
    return GetEffectiveBool(
      &GOPipeConfig::GetIndependentRelease,
      &GOPipeConfigNode::IsEffectiveIndependentRelease,
      (const GOSettingUnsigned GOConfig::*)nullptr);
  }

  bool GetEffectiveCompress() const {
    return GetEffectiveBool(
      &GOPipeConfig::GetCompress,
      &GOPipeConfigNode::GetEffectiveCompress,
      &GOConfig::LosslessCompression);
  }

  bool GetEffectiveAttackLoad() const {
    return GetEffectiveBool(
      &GOPipeConfig::GetAttackLoad,
      &GOPipeConfigNode::GetEffectiveAttackLoad,
      &GOConfig::AttackLoad);
  }

  bool GetEffectiveReleaseLoad() const {
    return GetEffectiveBool(
      &GOPipeConfig::GetReleaseLoad,
      &GOPipeConfigNode::GetEffectiveReleaseLoad,
      &GOConfig::ReleaseLoad);
  }

  bool GetEffectiveIgnorePitch() const {
    return GetEffectiveBool(
      &GOPipeConfig::IsIgnorePitch,
      &GOPipeConfigNode::GetEffectiveIgnorePitch,
      (const GOSettingUnsigned GOConfig::*)nullptr);
  }

  virtual void AddChild(GOPipeConfigNode *node) {}
  virtual unsigned GetChildCount() const { return 0; }
  virtual GOPipeConfigNode *GetChild(unsigned index) const { return nullptr; }
  virtual GOSampleStatistic GetStatistic() const;

  void ModifyManualTuning(float diff);
  void ModifyAutoTuningCorrection(float diff);
};

#endif
