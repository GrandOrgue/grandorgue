/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDRECORDER_H
#define GOSOUNDRECORDER_H

#include <wx/wx.h>
#include <wx/file.h>
#include "GOLock.h"

class GOSoundRecorder {
private:
	wxFile m_file;
	GOMutex m_lock;
	unsigned m_SampleRate;
	unsigned m_Channels;
	unsigned m_BytesPerSample;
	char buffer[300];

public:
	GOSoundRecorder();
	virtual ~GOSoundRecorder();

	void Open(wxString filename);
	bool IsOpen();
	void Close();
	void SetSampleRate(unsigned sample_rate);
	/* 1 = 8 bit, 2 = 16 bit, 3 = 24 bit, 4 = float */
	void SetBytesPerSample(unsigned value);

	void Write(float* data, unsigned count);
};

#endif
