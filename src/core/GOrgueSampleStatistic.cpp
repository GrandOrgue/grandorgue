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

#include "GOrgueSampleStatistic.h"

#include <math.h>

GOrgueSampleStatistic::GOrgueSampleStatistic() :
	m_Valid(false)
{
}

GOrgueSampleStatistic::~GOrgueSampleStatistic()
{
}

void GOrgueSampleStatistic::Prepare()
{
	if (!m_Valid)
		Init();
}


void GOrgueSampleStatistic::Init()
{
	m_Valid = true;
	m_MemorySize = 0;
	m_EndSegmentSize = 0;
	m_MinBitsPerSample = 0xff;
	m_MaxBitsPerSample = 0;
	m_UsedBits = 0;
	m_AllocatedSamples = 0;
}

void GOrgueSampleStatistic::Cumulate(const GOrgueSampleStatistic& stat)
{
	if (!stat.IsValid())
		return;
	Prepare();
	m_MemorySize += stat.m_MemorySize;
	m_EndSegmentSize += stat.m_EndSegmentSize;
	if (m_MinBitsPerSample > stat.m_MinBitsPerSample)
		m_MinBitsPerSample = stat.m_MinBitsPerSample;
	if (m_MaxBitsPerSample < stat.m_MaxBitsPerSample)
		m_MaxBitsPerSample = stat.m_MaxBitsPerSample;
	m_UsedBits += stat.m_UsedBits;
	m_AllocatedSamples += stat.m_AllocatedSamples;
}

bool GOrgueSampleStatistic::IsValid() const
{
	return m_Valid;
}

void GOrgueSampleStatistic::SetMemorySize(size_t size)
{
	Prepare();
	m_MemorySize = size;
}

size_t GOrgueSampleStatistic::GetMemorySize() const
{
	return m_MemorySize;
}

void GOrgueSampleStatistic::SetEndSegmentSize(size_t size)
{
	Prepare();
	m_EndSegmentSize = size;
}

void GOrgueSampleStatistic::SetBitsPerSample(unsigned bits, unsigned samples, unsigned max_value)
{
	if (bits < m_MinBitsPerSample)
		m_MinBitsPerSample = bits;
	if (bits > m_MaxBitsPerSample)
		m_MaxBitsPerSample = bits;
	m_UsedBits = samples * (size_t) max_value;
	m_AllocatedSamples = samples;
}

size_t GOrgueSampleStatistic::GetEndSegmentSize() const
{
	return m_EndSegmentSize;
}

unsigned GOrgueSampleStatistic::GetMinBitPerSample() const
{
	return m_MinBitsPerSample;
}

unsigned GOrgueSampleStatistic::GetMaxBitPerSample() const
{
	return m_MaxBitsPerSample;
}

float GOrgueSampleStatistic::GetUsedBits() const
{
	if (m_AllocatedSamples)
		return 1 + log2(m_UsedBits / (double)m_AllocatedSamples);
	else
		return 0;
}
