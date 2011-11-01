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

#include <wx/wx.h>
#include <wx/file.h>

#include "GOrgueMemoryPool.h"

GOrgueMemoryPool::GOrgueMemoryPool() :
	m_PoolStart(0),
	m_PoolPtr(0),
	m_PoolEnd(0),
	m_CacheStart(0),
	m_PoolSize(0),
	m_PoolLimit(0),
	m_PageSize(4096),
	m_CacheSize(0)
{
	InitPool();
}

GOrgueMemoryPool::~GOrgueMemoryPool()
{
	FreePool();
}

void *GOrgueMemoryPool::Alloc(unsigned length)
{
	void* data = PoolAlloc(length);
	if (data)
	{
		AddPoolAlloc(data);
		return data;
	}
	return malloc(length);
}

void GOrgueMemoryPool::Free(void* data)
{
	if (!data)
		return;
	if (m_PoolAllocs.count(data))
	{
		/* Pool memory is not individually freed */
		m_PoolAllocs.erase(m_PoolAllocs.find(data));
		return;
	}
	free(data);
}

void GOrgueMemoryPool::AddPoolAlloc(void* data)
{
	m_PoolAllocs.insert(data);
}

void* GOrgueMemoryPool::PoolAlloc(unsigned length)
{
	if (!m_PoolStart)
		return NULL;
	if (m_PoolPtr + length < m_PoolEnd)
	{
		void* data = m_PoolPtr;
		m_PoolPtr += length;
		return data;
	}
	GrowPool(length);
	if (m_PoolPtr + length < m_PoolEnd)
	{
		void* data = m_PoolPtr;
		m_PoolPtr += length;
		return data;
	}
	wxLogError(wxT("PoolAlloc failed: %d %d %08x %08x %08x"),
		   length, m_PoolSize, m_PoolStart, m_PoolPtr, m_PoolEnd);
	return NULL;
}

void *GOrgueMemoryPool::GetCacheData(unsigned long offset)
{
	if (m_CacheStart)
	{
		char* data = m_CacheStart + offset;
		AddPoolAlloc(data);
		return data;
	}
	return NULL;
}

bool GOrgueMemoryPool::SetCacheFile(wxFile& cache_file)
{
	bool result = false;
	FreePool();

	InitPool();
	return result;
}


void GOrgueMemoryPool::InitPool()
{
	m_PoolStart = 0;
	m_PoolSize = 0;
	m_PoolLimit = 0;
	
	m_PoolPtr = m_PoolStart;
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

void GOrgueMemoryPool::FreePool()
{
	m_PoolStart = 0;
	m_PoolSize = 0;
	m_PoolLimit = 0;
	
	m_CacheStart = 0;
	m_CacheSize = 0;
}

void GOrgueMemoryPool::GrowPool(unsigned long length)
{
	unsigned long new_size = m_PoolSize + 1000 * m_PageSize;
	if (new_size > m_PoolLimit)
		return;
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

