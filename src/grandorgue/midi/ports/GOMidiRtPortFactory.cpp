/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiRtPortFactory.h"

#include <wx/intl.h>
#include <wx/log.h>
#include <wx/regex.h>

#include <vector>

#include "GOMidiPortFactory.h"
#include "GOMidiRtInPort.h"
#include "GOMidiRtOutPort.h"

const wxString GOMidiRtPortFactory::PORT_NAME = wxT("Rt");

static std::vector<RtMidi::Api> apis;
static bool hasApisPopulated = false;

GOMidiRtPortFactory::GOMidiRtPortFactory()
  : m_AlsaDevnamePattern("([^:]+):([^:]+) ([0-9]+:[0-9]+)"),
    // Client Name:Port Name ClientNum:PortNum
    m_JackDevnamePattern(
      "Midi-Bridge:([^:]+):\\(([a-z]+)_([0-9]+)\\) ([^:-]+)(-[0-9]+)?$"),
    // Midi-Bridge:Client Name:(direction_num)) Port Name{-Num}
    m_WinMmDevnamePattern("([^:]+) ([0-9]+)$")
// Device Name PortNum
{
  if (!hasApisPopulated) {
    RtMidi::getCompiledApi(apis);
    hasApisPopulated = true;
  }
}

GOMidiRtPortFactory::~GOMidiRtPortFactory() {
  for (std::map<RtMidi::Api, RtMidiOut *>::iterator it = m_RtMidiOuts.begin();
       it != m_RtMidiOuts.end();
       it++) {
    RtMidiOut *const pRtMidi = it->second;

    if (pRtMidi != NULL) {
      it->second = NULL;
      delete pRtMidi;
    }
  }
  m_RtMidiOuts.clear();

  for (std::map<RtMidi::Api, RtMidiIn *>::iterator it = m_RtMidiIns.begin();
       it != m_RtMidiIns.end();
       it++) {
    RtMidiIn *const pRtMidi = it->second;

    if (pRtMidi != NULL) {
      it->second = NULL;
      delete pRtMidi;
    }
  }
  m_RtMidiIns.clear();
}

static GOMidiRtPortFactory *instance = NULL;

GOMidiRtPortFactory *GOMidiRtPortFactory::getInstance() {
  if (!instance)
    instance = new GOMidiRtPortFactory();
  return instance;
}

void GOMidiRtPortFactory::terminateInstance() {
  GOMidiRtPortFactory *oldInstance = instance;

  if (oldInstance) {
    instance = NULL;
    delete oldInstance;
  }
}

void GOMidiRtPortFactory::addMissingInDevices(
  GOMidi *midi,
  const GOPortsConfig &portsConfig,
  ptr_vector<GOMidiPort> &ports) {
  GOMidiPortFactory &portFactory(GOMidiPortFactory::getInstance());

  for (unsigned i = 0; i < apis.size(); i++)
    try {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);

      if (portsConfig.IsEnabled(PORT_NAME, apiName)) {
        RtMidiIn *&pRtMidiIn(m_RtMidiIns[api]);

        if (pRtMidiIn == NULL)
          pRtMidiIn
            = new RtMidiIn(api, std::string("GrandOrgueMidiIn-") + apiName);

        for (unsigned j = 0; j < pRtMidiIn->getPortCount(); j++) {
          const wxString deviceName
            = wxString::FromAscii(pRtMidiIn->getPortName(j).c_str());
          bool found = false;

          for (unsigned k = 0; k < ports.size(); k++) {
            GOMidiPort *const pOld = ports[k];

            if (pOld && pOld->IsEqualTo(PORT_NAME, apiName, deviceName)) {
              found = true;
              break;
            }
          }
          if (!found)
            ports.push_back(new GOMidiRtInPort(
              midi,
              api,
              deviceName,
              portFactory.ComposeDeviceName(PORT_NAME, apiName, deviceName)));
        }
      }
    } catch (RtMidiError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}

void GOMidiRtPortFactory::addMissingOutDevices(
  GOMidi *midi,
  const GOPortsConfig &portsConfig,
  ptr_vector<GOMidiPort> &ports) {
  GOMidiPortFactory &portFactory(GOMidiPortFactory::getInstance());

  for (unsigned i = 0; i < apis.size(); i++)
    try {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);

      if (portsConfig.IsEnabled(PORT_NAME, apiName)) {
        RtMidiOut *&pRtMidiOut(m_RtMidiOuts[api]);

        if (pRtMidiOut == NULL)
          pRtMidiOut
            = new RtMidiOut(api, std::string("GrandOrgueMidiOut-") + apiName);

        for (unsigned j = 0; j < pRtMidiOut->getPortCount(); j++) {
          const wxString deviceName
            = wxString::FromAscii(pRtMidiOut->getPortName(j).c_str());
          bool found = false;

          for (unsigned k = 0; k < ports.size(); k++) {
            const GOMidiPort *const pOld = ports[k];

            if (pOld && pOld->IsEqualTo(PORT_NAME, apiName, deviceName)) {
              found = true;
              break;
            }
          }
          if (!found)
            ports.push_back(new GOMidiRtOutPort(
              midi,
              api,
              deviceName,
              portFactory.ComposeDeviceName(PORT_NAME, apiName, deviceName)));
        }
      }
    } catch (RtMidiError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}

static bool hasApiNamesPopulated = false;
static std::vector<wxString> apiNames;

wxString GOMidiRtPortFactory::getApiName(RtMidi::Api api) {
  return wxString(RtMidi::getApiName(api));
}

const std::vector<wxString> &GOMidiRtPortFactory::getApis() {
  if (!hasApiNamesPopulated) {
    std::vector<RtMidi::Api> apiIndices;
    RtMidi::getCompiledApi(apiIndices);

    for (unsigned k = 0; k < apiIndices.size(); k++) {
      apiNames.push_back(getApiName(apiIndices[k]));
    }
    hasApiNamesPopulated = true;
  }
  return apiNames;
}

wxString GOMidiRtPortFactory::GetDefaultLogicalName(
  RtMidi::Api api, const wxString &deviceName, const wxString &fullName) {
  wxString logicalName;

  switch (api) {
  case RtMidi::Api::LINUX_ALSA:
    if (m_AlsaDevnamePattern.Matches(deviceName))
      logicalName = m_AlsaDevnamePattern.GetMatch(deviceName, 2);
    break;
  case RtMidi::Api::UNIX_JACK:
    if (m_JackDevnamePattern.Matches(deviceName))
      logicalName = m_JackDevnamePattern.GetMatch(deviceName, 4);
    break;
  case RtMidi::Api::WINDOWS_MM:
    if (m_WinMmDevnamePattern.Matches(deviceName))
      logicalName = m_WinMmDevnamePattern.GetMatch(deviceName, 1);
    break;
  default:
    break;
  }
  if (logicalName.IsEmpty())
    // by default logical name equals to the full physical name
    logicalName = fullName;
  else // add the api name to the logical name
    logicalName = GOMidiPortFactory::getInstance().ComposeDeviceName(
      PORT_NAME, getApiName(api), logicalName);
  return logicalName;
}

wxString GOMidiRtPortFactory::GetDefaultRegEx(
  RtMidi::Api api, const wxString &deviceName, const wxString &fullName) {
  wxString regEx;

  switch (api) {
  case RtMidi::Api::LINUX_ALSA:
    if (m_AlsaDevnamePattern.Matches(deviceName))
      // Client Name:Port Name
      regEx = wxString::Format(
        wxT("%s:%s"),
        m_AlsaDevnamePattern.GetMatch(deviceName, 1),
        m_AlsaDevnamePattern.GetMatch(deviceName, 2));
    break;
  case RtMidi::Api::UNIX_JACK:
    if (m_JackDevnamePattern.Matches(deviceName))
      // Midi-Bridge:Client Name:(direction_num)) Port Name
      regEx = wxString::Format(
        wxT("Midi-Bridge:%s:(%s_%s) %s"),
        m_JackDevnamePattern.GetMatch(deviceName, 1),
        m_JackDevnamePattern.GetMatch(deviceName, 2),
        m_JackDevnamePattern.GetMatch(deviceName, 3),
        m_JackDevnamePattern.GetMatch(deviceName, 4));
    break;
  case RtMidi::Api::WINDOWS_MM:
    if (m_WinMmDevnamePattern.Matches(deviceName))
      // Device Name
      regEx = m_WinMmDevnamePattern.GetMatch(deviceName, 1);
    break;
  default:
    break;
  }
  if (!regEx.IsEmpty())
    // add the api name to the regex
    regEx = GOMidiPortFactory::getInstance().ComposeDeviceName(
      PORT_NAME, getApiName(api), regEx);
  return regEx;
}
