/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GOSOUNDRECORDER_H
#define GOSOUNDRECORDER_H

#include <wx/wx.h>
#include <wx/file.h>

class GOSoundRecorder {
private:
	wxFile m_file;
	wxCriticalSection m_lock;
	unsigned m_SampleRate;

public:
	GOSoundRecorder();
	virtual ~GOSoundRecorder();

	void Open(wxString filename);
	bool IsOpen();
	void Close();
	void SetSampleRate(unsigned sample_rate);

	void Write(float* data, unsigned count);
};

#endif
