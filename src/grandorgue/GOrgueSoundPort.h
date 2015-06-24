/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GORGUESOUNDPORT_H
#define GORGUESOUNDPORT_H

#include "GOrgueSoundDevInfo.h"
#include <wx/string.h>
#include <vector>

class GOrgueSound;

class GOrgueSoundPort
{
protected:
	GOrgueSound* m_Sound;
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

public:
	GOrgueSoundPort(GOrgueSound* sound, wxString name);
	virtual ~GOrgueSoundPort();

	void Init(unsigned channels, unsigned sample_rate, unsigned samples_per_buffer, unsigned latency, unsigned index);
	virtual void Open() = 0;
	virtual void StartStream() = 0;
	virtual void Close() = 0;

	const wxString& GetName();

	static GOrgueSoundPort* create(GOrgueSound* sound, wxString name);
	static std::vector<GOrgueSoundDevInfo> getDeviceList();

	wxString getPortState();
};

#endif
