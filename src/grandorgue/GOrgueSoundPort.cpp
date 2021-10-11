/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include <wx/thread.h>

#include "GOrgueSoundPort.h"

#include "GOrgueSoundRtPort.h"
#include "GOrgueSoundPortaudioPort.h"
#include "GOrgueSoundJackPort.h"
#include "GOrgueSound.h"
#include <wx/intl.h>

const std::vector<wxString> GOrgueSoundPort::c_NoApis;

static const wxString NAME_DELIM = wxT(": ");
static const size_t NEME_DELIM_LEN = NAME_DELIM.length();

wxString GOrgueSoundPort::NameParser::nextComp()
{
  wxString res("");

  if (hasMore()) {
    size_t newPos = m_Name.find(NAME_DELIM, m_Pos);
    size_t compEnd;
    
    if (newPos != wxString::npos) {
      compEnd = newPos;
      newPos += NEME_DELIM_LEN;
    } else {
      compEnd = m_Name.length();
    }
    res = m_Name.substr(m_Pos, compEnd - m_Pos);
    m_Pos = newPos;
  }
  return res;
}

GOrgueSoundPort::GOrgueSoundPort(GOrgueSound* sound, wxString name) :
	m_Sound(sound),
	m_Index(0),
	m_IsOpen(false),
	m_Name(name),
	m_Channels(0),
	m_SamplesPerBuffer(0),
	m_SampleRate(0),
	m_Latency(0),
	m_ActualLatency(-1)
{
}

GOrgueSoundPort::~GOrgueSoundPort()
{
}

void GOrgueSoundPort::Init(unsigned channels, unsigned sample_rate, unsigned samples_per_buffer, unsigned latency, unsigned index)
{
	m_Index = index;
	m_Channels = channels;
	m_SampleRate = sample_rate;
	m_SamplesPerBuffer = samples_per_buffer;
	m_Latency = latency;
}

void GOrgueSoundPort::SetActualLatency(double latency)
{
	if (latency < m_SamplesPerBuffer / (double)m_SampleRate)
		latency = m_SamplesPerBuffer / (double)m_SampleRate;
	if (latency < 2 * m_SamplesPerBuffer / (double)m_SampleRate)
		latency += m_SamplesPerBuffer / (double)m_SampleRate;
	m_ActualLatency = latency * 1000;
}

bool GOrgueSoundPort::AudioCallback(float* outputBuffer, unsigned int nFrames)
{
	return m_Sound->AudioCallback(m_Index, outputBuffer, nFrames);
}

const wxString& GOrgueSoundPort::GetName()
{
	return m_Name;
}

void append_name(wxString const &nameComp, wxString &resName)
{
  if (! nameComp.IsEmpty()) {
    resName.Append(nameComp);
    resName.Append(NAME_DELIM);
  }
}

wxString GOrgueSoundPort::composeDeviceName(
  wxString const &subsysName,
  wxString const &apiName,
  wxString const &devName
)
{
  wxString resName;
  
  append_name(subsysName, resName);
  append_name(apiName, resName);
  append_name(devName, resName);
  return resName;
}

static bool has_subsystems_populated = false;
static std::vector<wxString> substystems;

const std::vector<wxString> & GOrgueSoundPort::getPortNames()
{
  if (! has_subsystems_populated)
  {
    substystems.push_back(GOrgueSoundPortaudioPort::PORT_NAME);
    substystems.push_back(GOrgueSoundRtPort::PORT_NAME);
    #if defined(GO_USE_JACK)
    substystems.push_back(GOrgueSoundJackPort::PORT_NAME);
    #endif
    has_subsystems_populated = true;
  }
  return substystems;
}

const std::vector<wxString> & GOrgueSoundPort::getApiNames(const wxString & portName)
{
  if (portName == GOrgueSoundPortaudioPort::PORT_NAME)
    return GOrgueSoundPortaudioPort::getApis();
  else if (portName == GOrgueSoundRtPort::PORT_NAME)
    return GOrgueSoundRtPort::getApis();
  else if (portName == GOrgueSoundJackPort::PORT_NAME)
    return GOrgueSoundJackPort::getApis();
  else // old-style name
    return c_NoApis;
}

enum {
  SUBSYS_PA_BIT = 1,
  SUBSYS_RT_BIT = 2,
  SUBSYS_JACK_BIT = 4
};

GOrgueSoundPort* GOrgueSoundPort::create(const GOrgueSoundPortsConfig &portsConfig, GOrgueSound* sound, wxString name)
{
  GOrgueSoundPort *port = NULL;
  NameParser parser(name);
  wxString subsysName = parser.nextComp();
  unsigned short subsysMask; // possible subsystems matching with the name
  
  if (subsysName == GOrgueSoundPortaudioPort::PORT_NAME)
    subsysMask = SUBSYS_PA_BIT;
  else if (subsysName == GOrgueSoundRtPort::PORT_NAME)
    subsysMask = SUBSYS_RT_BIT;
  else if (subsysName == GOrgueSoundJackPort::PORT_NAME)
    subsysMask = SUBSYS_JACK_BIT;
  else // old-style name
    subsysMask = SUBSYS_PA_BIT | SUBSYS_RT_BIT | SUBSYS_JACK_BIT;

  if (
    port == NULL && (subsysMask & SUBSYS_PA_BIT)
      && portsConfig.IsEnabled(GOrgueSoundPortaudioPort::PORT_NAME)
  ) port = GOrgueSoundPortaudioPort::create(portsConfig, sound, name);
  if (
    port == NULL && (subsysMask & SUBSYS_RT_BIT)
      && portsConfig.IsEnabled(GOrgueSoundRtPort::PORT_NAME)
  ) port = GOrgueSoundRtPort::create(portsConfig, sound, name);
  if (
    port == NULL && (subsysMask & SUBSYS_JACK_BIT)
      && portsConfig.IsEnabled(GOrgueSoundJackPort::PORT_NAME)
  ) port = GOrgueSoundJackPort::create(portsConfig, sound, name);
  return port;
}

std::vector<GOrgueSoundDevInfo> GOrgueSoundPort::getDeviceList(
  const GOrgueSoundPortsConfig &portsConfig
) {
  std::vector<GOrgueSoundDevInfo> result;
  
  if (portsConfig.IsEnabled(GOrgueSoundPortaudioPort::PORT_NAME))
    GOrgueSoundPortaudioPort::addDevices(portsConfig, result);
  if (portsConfig.IsEnabled(GOrgueSoundRtPort::PORT_NAME))
    GOrgueSoundRtPort::addDevices(portsConfig, result);
  if (portsConfig.IsEnabled(GOrgueSoundJackPort::PORT_NAME))
    GOrgueSoundJackPort::addDevices(portsConfig, result);
  return result;
}

void GOrgueSoundPort::terminate()
{
  GOrgueSoundPortaudioPort::terminate();
}

wxString GOrgueSoundPort::getPortState()
{
	if (m_ActualLatency < 0)
		return wxString::Format(_("%s: unknown"), GetName().c_str());
	else
		return wxString::Format(_("%s: %d ms"), GetName().c_str(), m_ActualLatency);
}
