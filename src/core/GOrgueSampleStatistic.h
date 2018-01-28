/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESAMPLESTATISTIC_H
#define GORGUESAMPLESTATISTIC_H

#include <stddef.h>

class GOrgueSampleStatistic
{
private:
	bool m_Valid;
	size_t m_MemorySize;
	size_t m_EndSegmentSize;
	unsigned m_MinBitsPerSample;
	unsigned m_MaxBitsPerSample;
	size_t m_AllocatedSamples;
	size_t m_UsedBits;

	void Prepare();

public:
	GOrgueSampleStatistic();
	~GOrgueSampleStatistic();

	void Init();
	void Cumulate(const GOrgueSampleStatistic& stat);

	void SetMemorySize(size_t size);
	void SetEndSegmentSize(size_t size);
	void SetBitsPerSample(unsigned bits, unsigned samples, unsigned max_value);

	bool IsValid() const;
	size_t GetMemorySize() const;
	size_t GetEndSegmentSize() const;
	unsigned GetMinBitPerSample() const;
	unsigned GetMaxBitPerSample() const;
	float GetUsedBits() const;
};

#endif

