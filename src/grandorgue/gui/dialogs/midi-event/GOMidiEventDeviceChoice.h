/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTDEVICECHOICE_H
#define GOMIDIEVENTDEVICECHOICE_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <wx/choice.h>

class GOMidiBasePatternList;
class GOMidiDeviceConfigList;
class GOMidiMap;
class GOPortsConfig;

/**
 * A visual control for selecting a MIDI device among a list of available
 * devices or "Any Device"
 */
class GOMidiEventDeviceChoice : public wxChoice {
private:
  // device id in the same order as they are in wxChoice
  std::vector<uint16_t> m_DeviceIdsByIndex;

  // map devices ids to the index in wxChoice
  std::unordered_map<uint16_t, unsigned> m_IndicesById;

  // add a device to the choice list
  void AddDevice(uint_fast16_t id, const wxString &name);

public:
  GOMidiEventDeviceChoice(wxWindow *pParent, wxWindowID id)
    : wxChoice(pParent, id) {}

  /**
   * Make a choice list from "Any Device", all available devices, enabled with
   *   portsConfig, and all referenced devices from patterns that are marked as
   *   MISSING
   * @param portsConfig - a config that enables or deisables MIDI ports
   * @param availableDevices - configured MIDI devices
   * @param m_MidiMap - a map between Device Id and Device Name
   * @param patterns - a list of MIDI events
   */
  void FillWithDevices(
    const GOPortsConfig &portsConfig,
    const GOMidiDeviceConfigList &availableDevices,
    const GOMidiMap &m_MidiMap,
    const GOMidiBasePatternList &patterns);

  /**
   * The Id of the selected device or "Any Device"
   */
  uint_fast16_t GetSelectedDeviceId() const;

  /**
   * Select the device with the id
   * @param deviceId an identifier of device to select
   */
  void SetSelectedDeviceId(uint_fast16_t deviceId);
};

#endif /* GOMIDIEVENTDEVICECHOICE_H */
