/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESOUNDPORTAUDIOPORT_H
#define GORGUESOUNDPORTAUDIOPORT_H

#include "GOrgueSoundPort.h"
#include "portaudio.h"

class GOrgueSoundPortaudioPort : public GOrgueSoundPort
{
private:
	PaStream* m_stream;

	static int Callback (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	static wxString getName(unsigned index);
	static wxString getLastError(PaError error);

public:
	static const wxString PORT_NAME;
  
	GOrgueSoundPortaudioPort(GOrgueSound* sound, wxString name);
	virtual ~GOrgueSoundPortaudioPort();

	void Open();
	void StartStream();
	void Close();

	static const std::vector<wxString> & getApis() { return c_NoApis; }
	static GOrgueSoundPort* create(const GOrgueSoundPortsConfig &portsConfig, GOrgueSound* sound, wxString name);
	static void addDevices(const GOrgueSoundPortsConfig &portsConfig, std::vector<GOrgueSoundDevInfo>& list);
	static void terminate();
};

#endif
