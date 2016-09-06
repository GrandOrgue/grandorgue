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

#include "GOrgueSampleStatistic.h"

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
}

void GOrgueSampleStatistic::Cumulate(const GOrgueSampleStatistic& stat)
{
	if (!stat.IsValid())
		return;
	Prepare();
	m_MemorySize += stat.m_MemorySize;
	m_EndSegmentSize += stat.m_EndSegmentSize;
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

size_t GOrgueSampleStatistic::GetEndSegmentSize() const
{
	return m_EndSegmentSize;
}
