/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDEVICECONFIGLIST_H
#define GOMIDIDEVICECONFIGLIST_H

#include <wx/string.h>

#include "GOMidiDeviceConfig.h"
#include "ptrvector.h"

class GOConfigReader;
class GOConfigWriter;

class GOMidiDeviceConfigList
// we can't use std::vector<GOMidiDeviceConfig> because it does not keep
// the references to it's elements unchanged
{
private:
  ptr_vector<GOMidiDeviceConfig> m_list;
  wxString m_GroupName;

public:
  GOMidiDeviceConfigList(const wxString &groupName = wxEmptyString)
    : m_GroupName(groupName) {}

  void Clear() { m_list.clear(); }

  GOMidiDeviceConfig::RefVector::const_iterator begin() const noexcept {
    return m_list.begin();
  }

  GOMidiDeviceConfig::RefVector::const_iterator end() const noexcept {
    return m_list.end();
  }

  GOMidiDeviceConfig *FindByLogicalName(const wxString &logicalName) const;

  /**
   * find device config by it's physical name.
   * If found, returns the reference to it in and sets m_PhysicalNameMatchedWith
   * and updates m_PortName and m_ApiName,
   * If not found, returns NULL
   **/
  GOMidiDeviceConfig *FindByPhysicalName(
    const wxString &physicalName,
    const wxString &portName,
    const wxString &apiName) const;

  /**
   * Remove the device from the list that has the logical name specified
   * and the physicalName is not in the protectList
   * @param logicalName - the logical device name to delete
   * @param protectList - the list of physical names to keep
   */
  void RemoveByLogicalNameOutOf(
    const wxString &logicalName,
    const GOMidiDeviceConfig::RefVector &protectList);

  /**
   * If devConfSrc.p_OutputDevice is set than fill devConfDst.p_OutputDevice
   * with a device with the same logical name in this list
   **/
  void MapOutputDevice(
    const GOMidiDeviceConfig &devConfSrc, GOMidiDeviceConfig &devConfDst) const;

  /**
   * Add a midi device config to the list
   * @param devConf the source config
   * @param outputList the list for finding output device by name
   * @return the reference to the new device
   **/
  GOMidiDeviceConfig *Append(
    const GOMidiDeviceConfig &devConf,
    const GOMidiDeviceConfigList *outputList = NULL);

  /**
   * Add a midi device to the list
   * @param logicalName the logical name of the midi device
   * @param regEx regex for matching with physical device
   * @param isEnabled is this device enabled
   * @param physicalName the physicalName this device is matched with
   * @return the reference to the new device
   **/
  GOMidiDeviceConfig *Append(
    const wxString &logicalName,
    const wxString &regEx,
    const wxString &portName,
    const wxString &apiName,
    bool isEnabled,
    const wxString &physicalName = wxEmptyString) {
    return Append(GOMidiDeviceConfig(
      logicalName, regEx, portName, apiName, isEnabled, physicalName));
  }

  /**
   * Load the list from the m_GroupName group of config file
   * @param cfg - the config file
   * @param outputMidiDevices
   *   when !=NULL then load input devices. Search outputDevice among this list
   *   when NULL then load output devices without additional search
   */
  void Load(
    GOConfigReader &cfg,
    const GOMidiDeviceConfigList *outputMidiDevices = NULL);

  /**
   * Save the list to the configuration file
   * @param cfg - the config file
   * @param isInput - whether to save input devices (they have more attributes)
   */
  void Save(GOConfigWriter &cfg, const bool isInput = false);
};

#endif /* GOMIDIDEVICECONFIGLIST_H */
