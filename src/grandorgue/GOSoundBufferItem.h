/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDBUFFERITEM_H
#define GOSOUNDBUFFERITEM_H

class GOSoundBufferItem
{
protected:
	unsigned m_SamplesPerBuffer;
	unsigned m_Channels;

public:
	GOSoundBufferItem(unsigned samples_per_buffer, unsigned channels) :
		m_SamplesPerBuffer(samples_per_buffer),
		m_Channels(channels)
	{
		m_Buffer = new float[m_SamplesPerBuffer * m_Channels];
	}
	virtual ~GOSoundBufferItem()
	{
		delete[] m_Buffer;
	}

	virtual void Finish() = 0;

	float* m_Buffer;

	unsigned GetSamplesPerBuffer()
	{
		return m_SamplesPerBuffer;
	}

	unsigned GetChannels()
	{
		return m_Channels;
	}
};

#endif
