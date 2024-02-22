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

void GOPipeConfigNode::SetParent(GOPipeConfigNode *parent) {
  m_parent = parent;
  if (m_parent)
    m_parent->AddChild(this);
}

void GOPipeConfigNode::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  r_OrganModel.RegisterSaveableObject(this);
  m_PipeConfig.Init(cfg, group, prefix);
}

void GOPipeConfigNode::Load(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  r_OrganModel.RegisterSaveableObject(this);
  m_PipeConfig.Load(
    cfg, group, prefix, m_parent && m_parent->GetEffectivePercussive());
}

wxString GOPipeConfigNode::GetEffectiveAudioGroup() const {
  if (m_PipeConfig.GetAudioGroup() != wxEmptyString)
    return m_PipeConfig.GetAudioGroup();
  if (m_parent)
    return m_parent->GetEffectiveAudioGroup();
  else
    return wxEmptyString;
}

float GOPipeConfigNode::GetEffectiveAmplitude() const {
  if (m_parent)
    return m_PipeConfig.GetAmplitude() * m_parent->GetEffectiveAmplitude()
      / 100.0;
  else
    return m_PipeConfig.GetAmplitude() / 100.0;
}

uint16_t GOPipeConfigNode::GetEffectiveReleaseTail() const {
  unsigned releaseTail = m_parent ? m_parent->GetEffectiveReleaseTail() : 0;
  const unsigned thisReleaseTail = m_PipeConfig.GetReleaseTail();

  // Set releaseTail as minimum between the parent release tail and this one
  if (thisReleaseTail && (!releaseTail || thisReleaseTail < releaseTail))
    releaseTail = thisReleaseTail;
  return releaseTail;
}

uint8_t GOPipeConfigNode::GetEffectiveUint8(
  int8_t (GOPipeConfig::*getThisValue)() const,
  uint8_t (GOPipeConfigNode::*getParentValue)() const,
  const GOSettingUnsigned GOConfig::*globalValue) const {
  int8_t thisValue = (m_PipeConfig.*getThisValue)();

  return thisValue >= 0 ? (unsigned)thisValue
    : m_parent          ? (m_parent->*getParentValue)()
                        : (m_config.*globalValue)();
}

GOSampleStatistic GOPipeConfigNode::GetStatistic() const {
  if (m_StatisticCallback)
    return m_StatisticCallback->GetStatistic();
  return GOSampleStatistic();
}

void GOPipeConfigNode::ModifyManualTuning(float diff) {
  m_PipeConfig.SetManualTuning(m_PipeConfig.GetManualTuning() + diff);
}

void GOPipeConfigNode::ModifyAutoTuningCorrection(float diff) {
  m_PipeConfig.SetAutoTuningCorrection(
    m_PipeConfig.GetAutoTuningCorrection() + diff);
}
