/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDeviceConfigList.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "go_limits.h"

GOMidiDeviceConfig *GOMidiDeviceConfigList::FindByLogicalName(
  const wxString &logicalName) const {
  GOMidiDeviceConfig *res = NULL;

  for (GOMidiDeviceConfig *pDevConf : m_list)
    if (pDevConf->GetLogicalName() == logicalName) {
      res = pDevConf;
      break;
    }
  return res;
}

GOMidiDeviceConfig *GOMidiDeviceConfigList::FindByPhysicalName(
  const wxString &physicalName,
  const wxString &portName,
  const wxString &apiName) const {
  GOMidiDeviceConfig *res = NULL;
  GOMidiDeviceConfig *candidate = NULL;

  for (GOMidiDeviceConfig *pDevConf : m_list) {
    const wxString confPhysicalName = pDevConf->GetPhysicalName();

    if (confPhysicalName == physicalName) { // hasAlreadyMatches
      res = pDevConf;
      break;
    }
    if (
      !candidate // physicalName has not yet matched
      && confPhysicalName.IsEmpty()
      && pDevConf->DoesMatch(physicalName) // devConf has not yet matched
    )
      candidate = pDevConf;
    // Even we have a candidate, we continue to search for already matched
  }
  if (!res && candidate) {
    candidate->SetPhysicalName(physicalName);
    res = candidate;
  }
  if (res) {
    res->SetPortName(portName);
    res->SetApiName(apiName);
  }
  return res;
}

void GOMidiDeviceConfigList::RemoveByLogicalNameOutOf(
  const wxString &logicalName,
  const GOMidiDeviceConfig::RefVector &protectList) {
  for (int i = m_list.size() - 1; i >= 0; i--) {
    GOMidiDeviceConfig *pDev = m_list[i];

    if (pDev->GetLogicalName() == logicalName) {
      bool isProtected = false;
      const wxString devPhysicalName = pDev->GetPhysicalName();

      if (!devPhysicalName.IsEmpty())
        for (GOMidiDeviceConfig *pProt : protectList)
          if (pProt->GetPhysicalName() == devPhysicalName) {
            isProtected = true;
            break;
          }
      if (!isProtected)
        m_list.erase(i);
    }
  }
}

void GOMidiDeviceConfigList::MapOutputDevice(
  const GOMidiDeviceConfig &devConfSrc, GOMidiDeviceConfig &devConfDst) const {
  devConfDst.p_OutputDevice = devConfSrc.p_OutputDevice
    ? FindByLogicalName(devConfSrc.p_OutputDevice->GetLogicalName())
    : NULL;
}

GOMidiDeviceConfig *GOMidiDeviceConfigList::Append(
  const GOMidiDeviceConfig &devConf, const GOMidiDeviceConfigList *outputList) {
  // At first, find the device with the same logical name
  GOMidiDeviceConfig *pDevConf = FindByLogicalName(devConf.GetLogicalName());
  bool toAdd = true;

  if (pDevConf && pDevConf->GetPhysicalName().IsEmpty()) {
    // the device is not matched. Replace it instead of adding a new one
    pDevConf->Assign(devConf);
    toAdd = false;
  } else {
    bool isDuplicate = pDevConf;

    pDevConf = new GOMidiDeviceConfig(devConf);
    if (isDuplicate)
    // construct an unique logical name
    {
      unsigned n = 0;

      do {
        pDevConf->SetLogicalName(
          wxString::Format(wxT("%s-%u"), devConf.GetLogicalName(), ++n));
      } while (FindByLogicalName(pDevConf->GetLogicalName()));
    }
  }

  if (outputList)
    // Map the output device against outputList
    outputList->MapOutputDevice(devConf, *pDevConf);
  if (toAdd)
    m_list.push_back(pDevConf);
  return pDevConf;
}

static const wxString COUNT = wxT("Count");
static const wxString DEVICE03D = wxT("Device%03d");

void GOMidiDeviceConfigList::Load(
  GOConfigReader &cfg, const GOMidiDeviceConfigList *outputMidiDevices) {
  unsigned count = cfg.ReadInteger(
    CMBSetting, m_GroupName, COUNT, 0, MAX_MIDI_DEVICES, false, 0);

  for (unsigned i = 1; i <= count; i++) {
    GOMidiDeviceConfig devConf;

    devConf.LoadDeviceConfig(
      cfg, m_GroupName, wxString::Format(DEVICE03D, i), outputMidiDevices);

    if (outputMidiDevices && !devConf.m_OutputDeviceName.IsEmpty()) {
      // load an input device
      devConf.p_OutputDevice
        = outputMidiDevices->FindByLogicalName(devConf.m_OutputDeviceName);
    }
    Append(devConf);
  }
}

void GOMidiDeviceConfigList::Save(GOConfigWriter &cfg, const bool isInput) {
  unsigned i = 0;

  for (GOMidiDeviceConfig *devConf : m_list)
    devConf->SaveDeviceConfig(
      cfg, m_GroupName, wxString::Format(DEVICE03D, ++i), isInput);
  if (i > MAX_MIDI_DEVICES)
    i = MAX_MIDI_DEVICES;
  cfg.WriteInteger(m_GroupName, COUNT, i);
}
