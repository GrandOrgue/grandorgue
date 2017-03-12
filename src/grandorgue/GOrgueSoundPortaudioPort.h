/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESOUNDPORTAUDIOPORT_H
#define GORGUESOUNDPORTAUDIOPORT_H

#include "GOrgueSoundPort.h"
#include "portaudio.h"

class GOrgueSoundPortaudioPort : public GOrgueSoundPort
{
private:
	PaStream* m_stream;

	static int Callback (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

public:
	static wxString getName(unsigned index);

	GOrgueSoundPortaudioPort(GOrgueSound* sound, wxString name);
	virtual ~GOrgueSoundPortaudioPort();

	void Open();
	void StartStream();
	void Close();

	static GOrgueSoundPort* create(GOrgueSound* sound, wxString name);
	static void addDevices(std::vector<GOrgueSoundDevInfo>& list);
};

#endif
