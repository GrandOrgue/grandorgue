#include "GOMidiRtFactory.h"
#include "GOMidiRtInPort.h"
#include "GOMidiRtOutPort.h"

#include <vector>
#include <wx/intl.h>
#include <wx/log.h>

static std::vector<RtMidi::Api> apis;
static bool hasApisPopulated = false;

GOMidiRtFactory::GOMidiRtFactory()
{
  if (! hasApisPopulated)
  {
    RtMidi::getCompiledApi(apis);
    hasApisPopulated = true;
  }
}

GOMidiRtFactory::~GOMidiRtFactory()
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

void GOMidiRtFactory::addMissingInDevices(GOMidi* midi, ptr_vector<GOMidiInPort>& ports)
{
  for(unsigned i = 0; i < apis.size(); i++)
    try
    {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);
      const wxString apiPrefix = wxString::FromAscii(apiName.c_str()) + ": ";
      RtMidiIn* &pRtMidiIn(m_RtMidiIns[api]);
      
      if (pRtMidiIn == NULL)
	pRtMidiIn = new RtMidiIn(api, std::string("GrandOrgueMidiIn-") + apiName);
      
      for (unsigned j = 0; j < pRtMidiIn->getPortCount(); j++)
      {
	wxString name = apiPrefix + wxString::FromAscii(pRtMidiIn->getPortName(j).c_str());
	bool found = false;

	for(unsigned k = 0; k < ports.size(); k++)
	  if (ports[k] && ports[k]->GetName() == name)
	    found = true;
	if (!found)
	  ports.push_back(new GOMidiRtInPort(midi, apiPrefix, name, api));
      }
    }
    catch (RtMidiError &e)
    {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}

void GOMidiRtFactory::addMissingOutDevices(GOMidi* midi, ptr_vector<GOMidiOutPort>& ports)
{
  for(unsigned i = 0; i < apis.size(); i++)
    try
    {
      const RtMidi::Api api = apis[i];
      const std::string apiName = RtMidi::getApiName(api);
      const wxString apiPrefix = wxString::FromAscii(apiName.c_str()) + ": ";
      RtMidiOut* &pRtMidiOut(m_RtMidiOuts[api]);
      
      if (pRtMidiOut == NULL)
	pRtMidiOut = new RtMidiOut(api, std::string("GrandOrgueMidiOut-") + apiName);
      
      for (unsigned j = 0; j < pRtMidiOut->getPortCount(); j++)
      {
	wxString name = apiPrefix + wxString::FromAscii(pRtMidiOut->getPortName(j).c_str());
	bool found = false;

	for(unsigned k = 0; k < ports.size(); k++)
	  if (ports[k] && ports[k]->GetName() == name)
	    found = true;
	if (!found)
	  ports.push_back(new GOMidiRtOutPort(midi, apiPrefix, name, api));
      }
    }
    catch (RtMidiError &e)
    {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtMidi error: %s"), error.c_str());
    }
}
