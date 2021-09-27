/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESOUNDRTPORT_H
#define GORGUESOUNDRTPORT_H

#include "GOrgueSoundPort.h"
#include "RtAudio.h"

class GOrgueSoundRtPort : public GOrgueSoundPort
{
private:
	RtAudio* m_rtApi;
	unsigned m_nBuffers;

	static int Callback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);

	static wxString getName(RtAudio* rtApi, unsigned index);

public:
	static const wxString PORT_NAME;
  
	// rtApi to be deleted in the destructor
	GOrgueSoundRtPort(GOrgueSound* sound, RtAudio* rtApi, wxString name);
	~GOrgueSoundRtPort();

	void Open();
	void StartStream();
	void Close();

	static const std::vector<wxString> & getApis();
	static GOrgueSoundPort* create(const GOrgueSoundPortsConfig &portsConfig, GOrgueSound* sound, wxString name);
	static void addDevices(const GOrgueSoundPortsConfig &portsConfig, std::vector<GOrgueSoundDevInfo>& list);
};

#endif
