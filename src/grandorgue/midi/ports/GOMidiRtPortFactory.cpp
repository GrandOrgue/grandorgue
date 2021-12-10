/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiRtPortFactory.h"
#include "GOMidiRtInPort.h"
#include "GOMidiRtOutPort.h"
#include "GOMidiPortFactory.h"

#include <vector>
#include <wx/intl.h>
#include <wx/log.h>

const wxString GOMidiRtPortFactory::PORT_NAME = wxT("Rt");

static std::vector<RtMidi::Api> apis;
static bool hasApisPopulated = false;

GOMidiRtPortFactory::GOMidiRtPortFactory()
{
  if (! hasApisPopulated)
  {
    RtMidi::getCompiledApi(apis);
    hasApisPopulated = true;
  }
}

GOMidiRtPortFactory::~GOMidiRtPortFactory()
{
  for (std::map<RtMidi::Api, RtMidiOut*>::iterator it = m_RtMidiOuts.begin(); it != m_RtMidiOuts.end(); it ++)
  {
    RtMidiOut* const pRtMidi = it->second;
    
    if (pRtMidi != NULL)
    {
      it->second = NULL;
      delete pRtMidi;
    }
  }
  m_RtMidiOuts.clear();
  
  for (std::map<RtMidi::Api, RtMidiIn*>::iterator it = m_RtMidiIns.begin(); it != m_RtMidiIns.end(); it ++)
  {
    RtMidiIn* const pRtMidi = it->second;
    
    if (pRtMidi != NULL)
    {
      it->second = NULL;
      delete pRtMidi;
    }
  }
  m_RtMidiIns.clear();
}

void GOMidiRtPortFactory::addMissingInDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiInPort>& ports)
{
  GOMidiPortFactory& portFactory(GOMidiPortFactory::getInstance());

  for (unsigned i = 0; i < apis.size(); i++)
    try
    {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);

      if (portsConfig.IsEnabled(PORT_NAME, apiName))
      {
	RtMidiIn* &pRtMidiIn(m_RtMidiIns[api]);

	if (pRtMidiIn == NULL)
	  pRtMidiIn = new RtMidiIn(api, std::string("GrandOrgueMidiIn-") + apiName);

	for (unsigned j = 0; j < pRtMidiIn->getPortCount(); j++)
	{
	  const wxString deviceName = wxString::FromAscii(pRtMidiIn->getPortName(j).c_str());
	  bool found = false;

	  for (unsigned k = 0; k < ports.size(); k++)
	  {
	    GOMidiInPort* const pOld = ports[k];

	    if (pOld && pOld->IsEqualTo(PORT_NAME, apiName, deviceName))
	    {
	      found = true;
	      break;
	    }
	  }
	  if (!found)
	    ports.push_back(
	      new GOMidiRtInPort(
		midi, api, deviceName,
		portFactory.ComposeDeviceName(PORT_NAME, apiName, deviceName)
	      )
	    );
	}
      }
    }
    catch (RtMidiError &e)
    {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}

void GOMidiRtPortFactory::addMissingOutDevices(GOMidi* midi, const GOPortsConfig& portsConfig, ptr_vector<GOMidiOutPort>& ports)
{
  GOMidiPortFactory& portFactory(GOMidiPortFactory::getInstance());

  for (unsigned i = 0; i < apis.size(); i++)
    try
    {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);

      if (portsConfig.IsEnabled(PORT_NAME, apiName))
      {
	RtMidiOut* &pRtMidiOut(m_RtMidiOuts[api]);

	if (pRtMidiOut == NULL)
	  pRtMidiOut = new RtMidiOut(api, std::string("GrandOrgueMidiOut-") + apiName);

	for (unsigned j = 0; j < pRtMidiOut->getPortCount(); j++)
	{
	  const wxString deviceName = wxString::FromAscii(pRtMidiOut->getPortName(j).c_str());
	  bool found = false;

	  for (unsigned k = 0; k < ports.size(); k++)
	  {
	    const GOMidiOutPort* const pOld = ports[k];

	    if (pOld && pOld->IsEqualTo(PORT_NAME, apiName, deviceName))
	    {
	      found = true;
	      break;
	    }
	  }
	  if (!found)
	    ports.push_back(
	      new GOMidiRtOutPort(
		midi, api, deviceName,
		portFactory.ComposeDeviceName(PORT_NAME, apiName, deviceName)
	      )
	    );
	}
      }
    }
    catch (RtMidiError &e)
    {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}

static bool hasApiNamesPopulated = false;
static std::vector<wxString> apiNames;

wxString GOMidiRtPortFactory::getApiName(RtMidi::Api api)
{
  return wxString(RtMidi::getApiName(api));
}

const std::vector<wxString> & GOMidiRtPortFactory::getApis()
{
  if (! hasApiNamesPopulated)
  {
    std::vector<RtMidi::Api> apiIndices;
    RtMidi::getCompiledApi(apiIndices);

    for (unsigned k = 0; k < apiIndices.size(); k++) {
      apiNames.push_back(getApiName(apiIndices[k]));
    }
    hasApiNamesPopulated = true;
  }
  return apiNames;
}
