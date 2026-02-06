/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventDeviceChoice.h"

#include "config/GOMidiDeviceConfigList.h"
#include "config/GOPortsConfig.h"
#include "midi/GOMidiMap.h"
#include "midi/events/GOMidiBasePatternList.h"
#include "midi/events/GOMidiEventPattern.h"

void GOMidiEventDeviceChoice::AddDevice(
  uint_fast16_t id, const wxString &name) {
  size_t index = m_DeviceIdsByIndex.size();
  int itemIndex = Append(name);

  assert((int)index == itemIndex);
  m_DeviceIdsByIndex.push_back(id);
  m_IndicesById[id] = static_cast<uint16_t>(index);
}

static const wxString WX_MISSING = _(" - MISSING");

void GOMidiEventDeviceChoice::FillWithDevices(
  const GOPortsConfig &portsConfig,
  const GOMidiDeviceConfigList &availableDevices,
  const GOMidiMap &m_MidiMap,
  const GOMidiBasePatternList &patterns) {
  Clear();
  AddDevice(GONameMap::ID_NOT_IN_FILE, _("Any device"));
  for (GOMidiDeviceConfig *pDevConf : availableDevices)
    if (
      portsConfig.IsEnabled(pDevConf->GetPortName(), pDevConf->GetApiName())
      && pDevConf->m_IsEnabled) {
      const wxString &logicalName = pDevConf->GetLogicalName();
      const uint_fast16_t existingDevId
        = m_MidiMap.GetDeviceIdByLogicalName(logicalName);

      // Don't show missing devices remaining in the configs, otherwise it will
      // confuse with "Any Device" that also has id=0
      if (existingDevId)
        AddDevice(existingDevId, logicalName);
    }
  for (unsigned l = patterns.GetEventCount(), i = 0; i < l; i++) {
    uint_fast16_t deviceId = patterns.GetBasePattern(i).deviceId;

    if (deviceId && !m_IndicesById.contains(deviceId))
      AddDevice(
        deviceId, m_MidiMap.GetDeviceLogicalNameById(deviceId) + WX_MISSING);
  }
}

uint_fast16_t GOMidiEventDeviceChoice::GetSelectedDeviceId() const {
  int selectedIndex = GetSelection();

  return selectedIndex >= 0 ? m_DeviceIdsByIndex[selectedIndex] : 0;
}

void GOMidiEventDeviceChoice::SetSelectedDeviceId(uint_fast16_t deviceId) {
  const auto it = m_IndicesById.find(deviceId);
  const auto index = it != m_IndicesById.end() ? it->second : 0;

  SetSelection((int)index);
}
