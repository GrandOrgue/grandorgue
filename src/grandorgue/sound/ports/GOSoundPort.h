/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDPORT_H
#define GOSOUNDPORT_H

#include "sound/GOSoundDevInfo.h"
#include "settings/GOPortsConfig.h"

#include <wx/string.h>
#include <vector>

class GOSound;

class GOSoundPort
{
protected:
  static const std::vector<wxString> c_NoApis; // empty Api list for subsystems not supporting them
  
  GOSound* m_Sound;
  unsigned m_Index;
  bool m_IsOpen;
  wxString m_Name;
  unsigned m_Channels;
  unsigned m_SamplesPerBuffer;
  unsigned m_SampleRate;
  unsigned m_Latency;
  int m_ActualLatency;

  void SetActualLatency(double latency);
  bool AudioCallback(float* outputBuffer, unsigned int nFrames);

  static wxString composeDeviceName(
    wxString const &subsysName,
    wxString const &apiName,
    wxString const &devName
  );

public:
  class NameParser
  {
  private:
    const wxString &m_Name;
    size_t m_Pos;

  public:
    NameParser(const wxString &name): m_Name(name), m_Pos(name.IsEmpty() ? wxString::npos : 0) { }
    NameParser(const NameParser &src): m_Name(src.m_Name), m_Pos(src.m_Pos) { }

    const wxString &GetOrigName() const { return m_Name; }
    bool hasMore() const { return m_Pos != wxString::npos; }

    const wxString GetRestName() const 
    { return hasMore() ? m_Name.substr(m_Pos) : wxT(""); }

    wxString nextComp();
  };
  
  GOSoundPort(GOSound* sound, wxString name);
  virtual ~GOSoundPort();

  void Init(unsigned channels, unsigned sample_rate, unsigned samples_per_buffer, unsigned latency, unsigned index);
  virtual void Open() = 0;
  virtual void StartStream() = 0;
  virtual void Close() = 0;

  const wxString& GetName();

  static const std::vector<wxString> & getPortNames();
  static const std::vector<wxString> & getApiNames(const wxString & portName);
  
  static std::vector<GOSoundDevInfo> getDeviceList(const GOPortsConfig &portsConfig);
  static GOSoundPort* create(const GOPortsConfig &portsConfig, GOSound* sound, wxString name);

  static void terminate();

  wxString getPortState();
};

#endif
