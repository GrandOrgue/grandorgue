/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESOUNDJACKPORT_H
#define GORGUESOUNDJACKPORT_H

#if defined(GO_USE_JACK)
#include <jack/jack.h>
#endif

#include "GOrgueSoundPort.h"

class GOrgueSoundJackPort : public GOrgueSoundPort
{
public:
	GOrgueSoundJackPort(GOrgueSound* sound, wxString name);
	~GOrgueSoundJackPort();

#if defined(GO_USE_JACK)
private:
	jack_client_t *m_JackClient = NULL;
	jack_port_t **m_JackOutputPorts = NULL;
	float *m_GoBuffer = NULL;
	bool m_IsOpen = false;
	bool m_IsStarted = false;
	
	static void JackLatencyCallback (jack_latency_callback_mode_t mode, void *data);
	static int JackProcessCallback(jack_nframes_t nFrames, void *data);
	static void JackShutdownCallback(void *data);
	
public:
	void Open();
	void StartStream();
#endif /* GO_USE_JACK */

public:
	void Close();
	static GOrgueSoundPort* create(GOrgueSound* sound, wxString name);
	static void addDevices(std::vector<GOrgueSoundDevInfo>& list);
};

#endif /* GORGUESOUNDJACKPORT_H */
