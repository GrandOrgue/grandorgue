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

#include "GOSoundOutputWorkItem.h"

#include "GOSoundReverb.h"
#include "mutex_locker.h"

GOSoundOutputWorkItem::GOSoundOutputWorkItem(unsigned channels, std::vector<float> scale_factors, unsigned samples_per_buffer) :
	GOSoundBufferItem(samples_per_buffer, channels),
	m_ScaleFactors(scale_factors),
	m_Outputs(),
	m_OutputCount(0),
	m_MeterInfo(channels),
	m_Reverb(0),
	m_Done(false)
{
	m_Reverb = new GOSoundReverb(m_Channels);
}

GOSoundOutputWorkItem::~GOSoundOutputWorkItem()
{
	if (m_Reverb)
		delete m_Reverb;
}

void GOSoundOutputWorkItem::SetOutputs(std::vector<GOSoundBufferItem*> outputs)
{
	m_Outputs = outputs;
	m_OutputCount = m_Outputs.size() * 2;
}

void GOSoundOutputWorkItem::Run()
{
	if (m_Done)
		return;
	GOMutexLocker locker(m_Mutex);
	if (m_Done)
		return;

	/* initialise the output buffer */
	std::fill(m_Buffer, m_Buffer + m_SamplesPerBuffer * m_Channels, 0.0f);

	for(unsigned i = 0; i < m_Channels; i++)
	{
		for(unsigned j = 0; j < m_OutputCount; j++)
		{
			float factor = m_ScaleFactors[i * m_OutputCount + j];
			if (factor == 0)
				continue;

			float* this_buff = m_Outputs[j / 2]->m_Buffer;
			m_Outputs[j / 2]->Finish();

			for (unsigned k = i, l = j % 2; k < m_SamplesPerBuffer * m_Channels; k += m_Channels, l+= 2)
				m_Buffer[k] += factor * this_buff[l];
		}
	}

	m_Reverb->Process(m_Buffer, m_SamplesPerBuffer);

	/* Clamp the output */
	const float CLAMP_MIN = -1.0f;
	const float CLAMP_MAX = 1.0f;
	for (unsigned k = 0, c = 0; k < m_SamplesPerBuffer * m_Channels; k++)
	{
		float f = std::min(std::max(m_Buffer[k], CLAMP_MIN), CLAMP_MAX);
		m_Buffer[k] = f;
		if (f > m_MeterInfo[c])
			m_MeterInfo[c] = f;

		c++;
		if (c >= m_Channels)
			c= 0;
	}

	m_Done = true;
}	

void GOSoundOutputWorkItem::Exec()
{
	Run();
}

void GOSoundOutputWorkItem::Finish()
{
	if (!m_Done)
		Run();
}

void GOSoundOutputWorkItem::Clear()
{
	m_Reverb->Reset();
	ResetMeterInfo();
}

void GOSoundOutputWorkItem::ResetMeterInfo()
{
	GOMutexLocker locker(m_Mutex);
	for(unsigned i = 0; i < m_MeterInfo.size(); i++)
		m_MeterInfo[i] = 0;
}

void GOSoundOutputWorkItem::Reset()
{
	GOMutexLocker locker(m_Mutex);
	m_Done = false;
}

unsigned GOSoundOutputWorkItem::GetGroup()
{
	return AUDIOOUTPUT;
}

unsigned GOSoundOutputWorkItem::GetCost()
{
	return 0;
}

void GOSoundOutputWorkItem::SetupReverb(GOrgueSettings& settings)
{
	m_Reverb->Setup(settings);
}

const std::vector<float>& GOSoundOutputWorkItem::GetMeterInfo()
{
	return m_MeterInfo;
}
