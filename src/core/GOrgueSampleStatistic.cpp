/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
