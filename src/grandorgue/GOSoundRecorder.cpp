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

#include "GOSoundRecorder.h"
#include "GOrgueSoundTypes.h"

GOSoundRecorder::GOSoundRecorder() :
        m_file(),
        m_lock(),
        m_SampleRate(0)
{
}

GOSoundRecorder::~GOSoundRecorder()
{
	Close();
}

void GOSoundRecorder::Open(wxString filename)
{
	struct_WAVE WAVE = {{'R','I','F','F'}, 0, {'W','A','V','E'}, {'f','m','t',' '}, 16, 3, 2, m_SampleRate, m_SampleRate * 8, 8, 32, {'d','a','t','a'}, 0};

	Close();

	wxCriticalSectionLocker locker(m_lock);

	m_file.Create(filename, true);
	if (!m_file.IsOpened())
	{
		::wxLogError(_("Unable to open file for writing"));
		return;
	}
	m_file.Write(&WAVE, sizeof(WAVE));
}

bool GOSoundRecorder::IsOpen()
{
	return m_file.IsOpened();
}

void GOSoundRecorder::Close()
{
	wxCriticalSectionLocker locker(m_lock);
	struct_WAVE WAVE = {{'R','I','F','F'}, 0, {'W','A','V','E'}, {'f','m','t',' '}, 16, 3, 2, m_SampleRate, m_SampleRate * 8, 8, 32, {'d','a','t','a'}, 0};

	if (!m_file.IsOpened())
		return;
	WAVE.ChunkSize = m_file.Tell() - 8;
	WAVE.Subchunk2Size = WAVE.ChunkSize - 36;
	m_file.Seek(0);
	m_file.Write(&WAVE, sizeof(WAVE));
	m_file.Close();
}

void GOSoundRecorder::SetSampleRate(unsigned sample_rate)
{
	m_SampleRate = sample_rate;
}

void GOSoundRecorder::Write(float* data, unsigned count)
{
       	wxCriticalSectionLocker locker(m_lock);
	if (!m_file.IsOpened())
		return;
	m_file.Write(data, count * sizeof(float));
}
