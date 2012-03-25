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
 * MA 02111-1307, USA.
 */

#include "GOSoundRecorder.h"
#include "GOrgueWaveTypes.h"

#pragma pack(push, 1)

struct struct_WAVE
{
	char ChunkID[4];
	wxUint32 ChunkSize;
	char Format[4];
	char Subchunk1ID[4];
	wxUint32 Subchunk1Size;
	wxUint16 AudioFormat;
	wxUint16 NumChannels;
	wxUint32 SampleRate;
	wxUint32 ByteRate;
	wxUint16 BlockAlign;
	wxUint16 BitsPerSample;
	char Subchunk2ID[4];
	wxUint32 Subchunk2Size;
};

#pragma pack(pop)


GOSoundRecorder::GOSoundRecorder() :
        m_file(),
        m_lock(),
        m_SampleRate(0),
	m_Channels(2),
	m_BytesPerSample(4)
{
}

GOSoundRecorder::~GOSoundRecorder()
{
	Close();
}

void GOSoundRecorder::Open(wxString filename)
{
	struct_WAVE WAVE = {
		{'R','I','F','F'}, 
		wxUINT32_SWAP_ON_BE(0), 
		{'W','A','V','E'}, 
		{'f','m','t',' '}, 
		wxUINT32_SWAP_ON_BE(16), 
		wxUINT16_SWAP_ON_BE(m_BytesPerSample == 4 ? 3 : 1), 
		wxUINT16_SWAP_ON_BE(m_Channels), 
		wxUINT32_SWAP_ON_BE(m_SampleRate), 
		wxUINT32_SWAP_ON_BE(m_SampleRate * m_BytesPerSample * m_Channels), 
		wxUINT16_SWAP_ON_BE(m_BytesPerSample * m_Channels), 
		wxUINT16_SWAP_ON_BE(8 * m_BytesPerSample), 
		{'d','a','t','a'}, 
		wxUINT32_SWAP_ON_BE(0)};

	Close();

	GOMutexLocker locker(m_lock);

	m_file.Create(filename, true);
	if (!m_file.IsOpened())
	{
		wxLogError(_("Unable to open file for writing"));
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
	GOMutexLocker locker(m_lock);
	struct_WAVE WAVE = {
		{'R','I','F','F'}, 
		wxUINT32_SWAP_ON_BE(0), 
		{'W','A','V','E'}, 
		{'f','m','t',' '}, 
		wxUINT32_SWAP_ON_BE(16), 
		wxUINT16_SWAP_ON_BE(m_BytesPerSample == 4 ? 3 : 1), 
		wxUINT16_SWAP_ON_BE(m_Channels), 
		wxUINT32_SWAP_ON_BE(m_SampleRate), 
		wxUINT32_SWAP_ON_BE(m_SampleRate  * m_BytesPerSample * m_Channels), 
		wxUINT16_SWAP_ON_BE(m_BytesPerSample * m_Channels), 
		wxUINT16_SWAP_ON_BE(8 * m_BytesPerSample), 
		{'d','a','t','a'}, 
		wxUINT32_SWAP_ON_BE(0)};

	if (!m_file.IsOpened())
		return;
	WAVE.ChunkSize = wxUINT32_SWAP_ON_BE(m_file.Tell() - 8);
	WAVE.Subchunk2Size = wxUINT32_SWAP_ON_BE((m_file.Tell() - 8) - 36);
	m_file.Seek(0);
	m_file.Write(&WAVE, sizeof(WAVE));
	m_file.Close();
}

void GOSoundRecorder::SetSampleRate(unsigned sample_rate)
{
	m_SampleRate = sample_rate;
}

void GOSoundRecorder::SetBytesPerSample(unsigned value)
{
	if (value < 1 || value > 4)
		value = 4;
	m_BytesPerSample = value;
}

inline int float_to_fixed(float f, unsigned fractional_bits)
{
	assert(fractional_bits > 0);
	int max_val = 1 << fractional_bits;
	int f_exp = f * max_val;
	if (f_exp < -max_val)
		return -max_val;
	if (f_exp > max_val - 1)
		return max_val - 1;
	return f_exp;
}

void GOSoundRecorder::Write(float* data, unsigned count)
{
	GOMutexLocker locker(m_lock);
	if (!m_file.IsOpened())
		return;
	if (m_BytesPerSample == 4)
		m_file.Write(data, count * sizeof(float));
	else if (m_BytesPerSample == 1)
	{
		unsigned char* buf = (unsigned char*)buffer;
		unsigned size = sizeof(buffer) / sizeof(unsigned char);
		unsigned pos = 0;
		for(unsigned i = 0; i < count; i++)
		{
			if (pos >= size)
			{
				m_file.Write(buf, pos * sizeof(unsigned char));
				pos = 0;
			}
			buf[pos++] = (unsigned char)(float_to_fixed(data[i], 7) + 128);
		}
		m_file.Write(buf, pos * sizeof(unsigned char));
	}
	else if (m_BytesPerSample == 2)
	{
		wxInt16* buf = (wxInt16*)buffer;
		unsigned size = sizeof(buffer) / sizeof(wxInt16);
		unsigned pos = 0;
		for(unsigned i = 0; i < count; i++)
		{
			if (pos >= size)
			{
				m_file.Write(buf, pos * sizeof(wxInt16));
				pos = 0;
			}
			buf[pos++] = wxINT16_SWAP_ON_BE(float_to_fixed(data[i], 15));
		}
		m_file.Write(buf, pos * sizeof(wxInt16));
	}
	else if (m_BytesPerSample == 3)
	{
		GO_Int24* buf = (GO_Int24*)buffer;
		unsigned size = sizeof(buffer) / sizeof(GO_Int24);
		unsigned pos = 0;
		for(unsigned i = 0; i < count; i++)
		{
			if (pos >= size)
			{
				m_file.Write(buf, pos * sizeof(GO_Int24));
				pos = 0;
			}
			buf[pos++] = IntToGOInt24(float_to_fixed(data[i], 23));
		}
		m_file.Write(buf, pos * sizeof(GO_Int24));
	}
}
