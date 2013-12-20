/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueWavPack.h"

#include "GOrgueMemoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GOrgueWavPack::GOrgueWavPack(const char* data, unsigned length) :
	m_data(data),
	m_length(length),
	m_Samples(0),
	m_SamplesLen(0),
	m_Wrapper(0),
	m_WrapperLen(0),
	m_pos(0),
	m_OrigDataLen(0),
	m_context(0)
{
}

GOrgueWavPack::~GOrgueWavPack()
{
	if (m_Samples)
		free(m_Samples);
	if (m_Wrapper)
		free(m_Wrapper);
	if (m_context)
		WavpackCloseFile(m_context);
}

bool GOrgueWavPack::IsWavPack()
{
	return m_length > 10 && !memcmp(m_data, "wvpk", 4);
}

void GOrgueWavPack::GetSamples(char*& data, unsigned& len)
{
	data = m_Samples;
	len = m_SamplesLen;
	m_Samples = NULL;
}

void GOrgueWavPack::GetWrapper(char*& data, unsigned& len)
{
	data = m_Wrapper;
	len = m_WrapperLen;
}

unsigned GOrgueWavPack::GetOrigDataLen()
{
	return m_OrigDataLen;
}

bool GOrgueWavPack::Unpack()
{
	m_context = WavpackOpenFileInputEx(&m_Reader, this, NULL, NULL, OPEN_WRAPPER, 0);
	if (!m_context)
		return false;

	unsigned header = WavpackGetWrapperBytes(m_context);
	if (!header)
		return false;

	unsigned channels = WavpackGetNumChannels (m_context);
	unsigned samples = WavpackGetNumSamples (m_context);
	m_SamplesLen = channels * samples * 4;
	m_Samples = (char*)malloc(m_SamplesLen);
	if  (!m_Samples)
		throw GOrgueOutOfMemory();
	unsigned res = WavpackUnpackSamples(m_context, (int32_t*)m_Samples, samples);
	if (res != samples)
		return false;

	m_OrigDataLen = channels * samples * WavpackGetBytesPerSample(m_context);

	WavpackSeekTrailingWrapper(m_context);
	unsigned trailer = WavpackGetWrapperBytes(m_context) - header;
	
	m_WrapperLen = header + trailer;
	m_Wrapper = (char*)malloc(m_WrapperLen);
	if (!m_Wrapper)
		throw GOrgueOutOfMemory();
	memcpy(m_Wrapper, WavpackGetWrapperData (m_context), m_WrapperLen);
	WavpackFreeWrapper (m_context);

	WavpackCloseFile(m_context);
	m_context = NULL;

	return true;
}

uint32_t GOrgueWavPack::GetLength (void *id)
{
	return ((GOrgueWavPack*)id)->GetLength();
}

int32_t GOrgueWavPack::ReadBytes (void *id, void *data, int32_t bcount)
{
	return ((GOrgueWavPack*)id)->ReadBytes(data, bcount);
}

int GOrgueWavPack::PushBackByte (void *id, int c)
{
	return ((GOrgueWavPack*)id)->PushBackByte(c);
}

uint32_t GOrgueWavPack::GetPos(void *id)
{
	return ((GOrgueWavPack*)id)->GetPos();
}

int GOrgueWavPack::CanSeek(void *id)
{
	return ((GOrgueWavPack*)id)->CanSeek();
}

int GOrgueWavPack::SetPosAbs(void *id, uint32_t pos)
{
	return ((GOrgueWavPack*)id)->SetPosAbs(pos);
}

int GOrgueWavPack::SetPosRel(void *id, int32_t delta, int mode)
{
	return ((GOrgueWavPack*)id)->SetPosRel(delta, mode);
}

uint32_t GOrgueWavPack::GetLength()
{
	return m_length;
}

int32_t GOrgueWavPack::ReadBytes (void *data, int32_t bcount)
{
	if (m_pos + bcount > m_length)
		bcount = m_length - m_pos;
	memcpy(data, m_data + m_pos, bcount);
	m_pos += bcount;
	return bcount;
}

int GOrgueWavPack::PushBackByte (int c)
{
	if (m_pos > 0 && m_data[m_pos - 1] == c)
	{
		m_pos--;
		return c;
	}
	return EOF;
}

uint32_t GOrgueWavPack::GetPos()
{
	return m_pos;
}

int GOrgueWavPack::CanSeek()
{
	return 1;
}

int GOrgueWavPack::SetPosAbs(uint32_t pos)
{
	if (pos < m_length)
	{
		m_pos = pos;
		return 0;
	}
	return -1;
}

int GOrgueWavPack::SetPosRel(int32_t delta, int mode)
{
	switch(mode)
	{
	case SEEK_SET:
		return SetPosAbs(delta);
	case SEEK_CUR:
		return SetPosAbs(m_pos + delta);
	case SEEK_END:
		return SetPosAbs(m_length + delta);
	default:
		return -1;
	}
}

WavpackStreamReader GOrgueWavPack::m_Reader = {
	read_bytes: ReadBytes, 
	get_pos: GetPos, 
	set_pos_abs: SetPosAbs, 
	set_pos_rel: SetPosRel, 
	push_back_byte: PushBackByte, 
	get_length: GetLength, 
	can_seek: CanSeek,
	write_bytes: NULL
};
