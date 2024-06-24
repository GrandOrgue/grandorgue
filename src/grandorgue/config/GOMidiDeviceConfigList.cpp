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
static const wxString DEVICE03D_ENABLED = wxT("Device%03dEnabled");
static const wxString DEVICE03D_REGEX = wxT("Device%03dRegEx");
static const wxString DEVICE03D_PORT_NAME = wxT("Device%03dPortName");
static const wxString DEVICE03D_API_NAME = wxT("Device%03dApiName");
static const wxString DEVICE03D_SHIFT = wxT("Device%03dShift");
static const wxString DEVICE03D_OUTPUT_DEVICE = wxT("Device%03dOutputDevice");

void GOMidiDeviceConfigList::Load(
  GOConfigReader &cfg, const GOMidiDeviceConfigList *outputMidiDevices) {
  unsigned count = cfg.ReadInteger(
    CMBSetting, m_GroupName, COUNT, 0, MAX_MIDI_DEVICES, false, 0);

  for (unsigned i = 1; i <= count; i++) {
    GOMidiDeviceConfig *const devConf = Append(
      cfg.ReadString(CMBSetting, m_GroupName, wxString::Format(DEVICE03D, i)),
      // logicalName
      cfg.ReadString(
        CMBSetting,
        m_GroupName,
        wxString::Format(DEVICE03D_REGEX, i),
        false), // regEx
      cfg.ReadString(
        CMBSetting,
        m_GroupName,
        wxString::Format(DEVICE03D_PORT_NAME, i),
        false), // portName
      cfg.ReadString(
        CMBSetting,
        m_GroupName,
        wxString::Format(DEVICE03D_API_NAME, i),
        false), // apiName
      cfg.ReadBoolean(
        CMBSetting,
        m_GroupName,
        wxString::Format(DEVICE03D_ENABLED, i)) // isEnabled
    );

    if (outputMidiDevices) // load an input device
    {
      devConf->m_ChannelShift = cfg.ReadInteger(
        CMBSetting, m_GroupName, wxString::Format(DEVICE03D_SHIFT, i), 0, 15);

      const wxString outDevName = cfg.ReadString(
        CMBSetting,
        m_GroupName,
        wxString::Format(DEVICE03D_OUTPUT_DEVICE, i),
        false);

      if (!outDevName.IsEmpty())
        devConf->p_OutputDevice
          = outputMidiDevices->FindByLogicalName(outDevName);
    }
  }
}

void GOMidiDeviceConfigList::Save(GOConfigWriter &cfg, const bool isInput) {
  unsigned i = 0;

  for (GOMidiDeviceConfig *devConf : m_list) {
    i++;
    cfg.WriteString(
      m_GroupName, wxString::Format(DEVICE03D, i), devConf->GetLogicalName());
    cfg.WriteString(
      m_GroupName, wxString::Format(DEVICE03D_REGEX, i), devConf->GetRegEx());
    cfg.WriteString(
      m_GroupName,
      wxString::Format(DEVICE03D_PORT_NAME, i),
      devConf->GetPortName());
    cfg.WriteString(
      m_GroupName,
      wxString::Format(DEVICE03D_API_NAME, i),
      devConf->GetApiName());
    cfg.WriteBoolean(
      m_GroupName,
      wxString::Format(DEVICE03D_ENABLED, i),
      devConf->m_IsEnabled);
    if (isInput) {
      cfg.WriteInteger(
        m_GroupName,
        wxString::Format(DEVICE03D_SHIFT, i),
        devConf->m_ChannelShift);
      if (devConf->p_OutputDevice)
        cfg.WriteString(
          m_GroupName,
          wxString::Format(DEVICE03D_OUTPUT_DEVICE, i),
          devConf->p_OutputDevice->GetLogicalName());
    }
  }
  if (i > MAX_MIDI_DEVICES)
    i = MAX_MIDI_DEVICES;
  cfg.WriteInteger(m_GroupName, COUNT, i);
}
