/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeConfigNode.h"

#include "config/GOConfig.h"
#include "model/GOOrganModel.h"

#include "GOSampleStatistic.h"
#include "GOStatisticCallback.h"

GOPipeConfigNode::GOPipeConfigNode(
  GOPipeConfigNode *parent,
  GOOrganModel &organModel,
  GOPipeUpdateCallback *callback,
  GOStatisticCallback *statistic)
  : r_OrganModel(organModel),
    m_config(organModel.GetConfig()),
    m_parent(parent),
    m_PipeConfig(organModel, callback),
    m_StatisticCallback(statistic),
    m_Name() {
  if (m_parent)
    m_parent->AddChild(this);
}

GOPipeConfigNode::~GOPipeConfigNode() {}

void GOPipeConfigNode::SetParent(GOPipeConfigNode *parent) {
  m_parent = parent;
  if (m_parent)
    m_parent->AddChild(this);
}

const wxString &GOPipeConfigNode::GetName() { return m_Name; }

void GOPipeConfigNode::SetName(wxString name) { m_Name = name; }

void GOPipeConfigNode::Init(
  GOConfigReader &cfg, wxString group, wxString prefix) {
  r_OrganModel.RegisterSaveableObject(this);
  m_PipeConfig.Init(cfg, group, prefix);
}

void GOPipeConfigNode::Load(
  GOConfigReader &cfg, wxString group, wxString prefix) {
  r_OrganModel.RegisterSaveableObject(this);
  m_PipeConfig.Load(cfg, group, prefix);
}

void GOPipeConfigNode::Save(GOConfigWriter &cfg) { m_PipeConfig.Save(cfg); }

GOPipeConfig &GOPipeConfigNode::GetPipeConfig() { return m_PipeConfig; }

void GOPipeConfigNode::ModifyManualTuning(float diff) {
  m_PipeConfig.SetManualTuning(m_PipeConfig.GetManualTuning() + diff);
}

void GOPipeConfigNode::ModifyAutoTuningCorrection(float diff) {
  m_PipeConfig.SetAutoTuningCorrection(
    m_PipeConfig.GetAutoTuningCorrection() + diff);
}

float GOPipeConfigNode::GetEffectiveAmplitude() {
  if (m_parent)
    return m_PipeConfig.GetAmplitude() * m_parent->GetEffectiveAmplitude()
      / 100.0;
  else
    return m_PipeConfig.GetAmplitude() / 100.0;
}

float GOPipeConfigNode::GetEffectiveGain() {
  if (m_parent)
    return m_PipeConfig.GetGain() + m_parent->GetEffectiveGain();
  else
    return m_PipeConfig.GetGain();
}

float GOPipeConfigNode::GetEffectivePitchTuning() const {
  float pitchTuning = m_PipeConfig.GetPitchTuning();

  if (m_parent)
    pitchTuning += m_parent->GetEffectivePitchTuning();
  return pitchTuning;
}

float GOPipeConfigNode::GetEffectivePitchCorrection() const {
  float pitchCorrection = m_PipeConfig.GetPitchCorrection();

  if (m_parent)
    pitchCorrection += m_parent->GetEffectivePitchCorrection();
  return pitchCorrection;
}

float GOPipeConfigNode::GetEffectiveManualTuning() const {
  float cents = m_PipeConfig.GetManualTuning();

  if (m_parent)
    cents += m_parent->GetEffectiveManualTuning();
  return cents;
}

float GOPipeConfigNode::GetEffectiveAutoTuningCorection() const {
  float cents = m_PipeConfig.GetAutoTuningCorrection();

  if (m_parent)
    cents += m_parent->GetEffectiveAutoTuningCorection();
  return cents;
}

unsigned GOPipeConfigNode::GetEffectiveDelay() const {
  if (m_parent)
    return m_PipeConfig.GetDelay() + m_parent->GetEffectiveDelay();
  else
    return m_PipeConfig.GetDelay();
}

wxString GOPipeConfigNode::GetEffectiveAudioGroup() const {
  if (m_PipeConfig.GetAudioGroup() != wxEmptyString)
    return m_PipeConfig.GetAudioGroup();
  if (m_parent)
    return m_parent->GetEffectiveAudioGroup();
  else
    return wxEmptyString;
}

unsigned GOPipeConfigNode::GetEffectiveBitsPerSample() const {
  if (m_PipeConfig.GetBitsPerSample() != -1)
    return m_PipeConfig.GetBitsPerSample();
  if (m_parent)
    return m_parent->GetEffectiveBitsPerSample();
  else
    return m_config.BitsPerSample();
}

bool GOPipeConfigNode::GetEffectiveCompress() const {
  if (m_PipeConfig.GetCompress() != -1)
    return m_PipeConfig.GetCompress() ? true : false;
  if (m_parent)
    return m_parent->GetEffectiveCompress();
  else
    return m_config.LosslessCompression();
}

unsigned GOPipeConfigNode::GetEffectiveLoopLoad() const {
  if (m_PipeConfig.GetLoopLoad() != -1)
    return m_PipeConfig.GetLoopLoad();
  if (m_parent)
    return m_parent->GetEffectiveLoopLoad();
  else
    return m_config.LoopLoad();
}

bool GOPipeConfigNode::GetEffectiveAttackLoad() const {
  const int thisConfigValue = m_PipeConfig.GetAttackLoad();

  return thisConfigValue != -1 ? (bool)thisConfigValue
    : m_parent                 ? m_parent->GetEffectiveAttackLoad()
                               : m_config.AttackLoad();
}

bool GOPipeConfigNode::GetEffectiveReleaseLoad() const {
  const int thisConfigValue = m_PipeConfig.GetReleaseLoad();

  return thisConfigValue != -1 ? (bool)thisConfigValue
    : m_parent                 ? m_parent->GetEffectiveReleaseLoad()
                               : m_config.ReleaseLoad();
}

unsigned GOPipeConfigNode::GetEffectiveChannels() const {
  if (m_PipeConfig.GetChannels() != -1)
    return m_PipeConfig.GetChannels();
  if (m_parent)
    return m_parent->GetEffectiveChannels();
  else
    return m_config.LoadChannels();
}

GOSampleStatistic GOPipeConfigNode::GetStatistic() const {
  if (m_StatisticCallback)
    return m_StatisticCallback->GetStatistic();
  return GOSampleStatistic();
}

bool GOPipeConfigNode::GetEffectiveIgnorePitch() const {
  const int thisConfigValue = m_PipeConfig.IsIgnorePitch();

  return thisConfigValue != -1
    ? (thisConfigValue)
    : m_parent && m_parent->GetEffectiveIgnorePitch();
}

unsigned GOPipeConfigNode::GetEffectiveReleaseTail() const {
  unsigned releaseTail = m_parent ? m_parent->GetEffectiveReleaseTail() : 0;
  const unsigned thisReleaseTail = m_PipeConfig.GetReleaseTail();

  // Set releaseTail as minimum between the parent release tail and this one
  if (thisReleaseTail && (!releaseTail || thisReleaseTail < releaseTail))
    releaseTail = thisReleaseTail;
  return releaseTail;
}

unsigned GOPipeConfigNode::GetChildCount() { return 0; }

GOPipeConfigNode *GOPipeConfigNode::GetChild(unsigned index) { return NULL; }

void GOPipeConfigNode::AddChild(GOPipeConfigNode *node) {}
