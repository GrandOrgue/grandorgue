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

#include "GOSoundTremulantWorkItem.h"

#include "GOSoundEngine.h"

GOSoundTremulantWorkItem::GOSoundTremulantWorkItem(GOSoundEngine& sound_engine, unsigned samples_per_buffer) :
	m_engine(sound_engine),
	m_Volume(0),
	m_SamplesPerBuffer(samples_per_buffer),
	m_Done(false)
{
}

void GOSoundTremulantWorkItem::Reset()
{
	GOMutexLocker locker(m_Mutex);
	m_Done = false;
}

void GOSoundTremulantWorkItem::Clear()
{
	m_Samplers.Clear();
}

void GOSoundTremulantWorkItem::Add(GO_SAMPLER* sampler)
{
	m_Samplers.Put(sampler);
}

unsigned GOSoundTremulantWorkItem::GetCost()
{
	return (unsigned)-1;
}

void GOSoundTremulantWorkItem::Run()
{
	if (m_Done)
		return;

	GOMutexLocker locker(m_Mutex);

	if (m_Done)
		return;

	m_Samplers.Move();
	if (m_Samplers.Peek() == NULL)
	{
		m_Volume = 1;
		m_Done = true;
		return;
	}

	float output_buffer[m_SamplesPerBuffer * 2];
	std::fill(output_buffer, output_buffer + m_SamplesPerBuffer * 2, 0.0f);
	output_buffer[2 * m_SamplesPerBuffer - 1] = 1.0f;
	for (GO_SAMPLER* sampler = m_Samplers.Get(); sampler; sampler = m_Samplers.Get())
	{
		bool keep;
		keep = m_engine.ProcessSampler(output_buffer, sampler, m_SamplesPerBuffer, 1);

		if (!keep)
		{
			m_engine.ReturnSampler(sampler);
		}
		else
			m_Samplers.Put(sampler);

	}
	m_Volume = output_buffer[2 * m_SamplesPerBuffer - 1];
	m_Done = true;
}
