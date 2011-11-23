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
#ifdef linux
#include <sys/mman.h>
#endif

#include "GOrgueMemoryPool.h"

GOrgueMemoryPool::GOrgueMemoryPool() :
	m_PoolStart(0),
	m_PoolPtr(0),
	m_PoolEnd(0),
	m_CacheStart(0),
	m_PoolSize(0),
	m_PoolLimit(0),
	m_PageSize(4096),
	m_CacheSize(0),
	m_dummy(0)
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

void * GOrgueMemoryPool::Realloc(void* data, unsigned old_length, unsigned new_length)
{
	if (m_PoolAllocs.count(data))
	{
		if (m_PoolPtr - old_length != data)
		{
			wxLogWarning(_("Realloc of non-top element failed"));
			return data;
		}
		m_PoolPtr = m_PoolPtr - old_length + new_length;
		return data;
	}
	void* new_data = realloc(data, new_length);
	if (new_data)
		return new_data;
	return data;
}


void GOrgueMemoryPool::AddPoolAlloc(void* data)
{
	m_PoolAllocs.insert(data);
}

void* GOrgueMemoryPool::PoolAlloc(unsigned length)
{
	if (!m_PoolStart)
		return NULL;
	if (!length)
		length++;
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

void *GOrgueMemoryPool::GetCacheData(unsigned long offset, unsigned length)
{
	if (!length)
		return NULL;
	if (m_CacheStart)
	{
		char* data = m_CacheStart + offset;
		for (unsigned i = 0; i < length; i+= 512)
			m_dummy += data[i];
		if (length)
			m_dummy += data[length - 1];
		AddPoolAlloc(data);
		return data;
	}
	return NULL;
}

void GOrgueMemoryPool::FreeCacheFile()
{
	FreePool();
	InitPool();
}

bool GOrgueMemoryPool::SetCacheFile(wxFile& cache_file)
{
	bool result = false;
	FreePool();

#ifdef linux
	m_CacheSize = cache_file.Length();
	m_CacheStart = (char*)mmap(NULL, m_CacheSize, PROT_READ, MAP_SHARED, cache_file.fd(), 0);
	if (m_CacheStart == MAP_FAILED)
	{
		m_CacheStart = 0;
		m_CacheSize = 0;
	}
	else
		result = true;
	
#endif

	InitPool();
	return result;
}


void GOrgueMemoryPool::InitPool()
{
	m_PoolStart = 0;
	m_PoolSize = 0;
	m_PoolLimit = 0;
	
#ifdef linux
	m_PageSize = sysconf(_SC_PAGESIZE);
	/* We reserve virtual address and add backing memory only, if the 
	   memory region is needed.
	   
	   On 32 bit, limit to 2 GB address space (so 1 GB is left for the rest of GO)
	   On 64 bit, we reserve the size of the physical memory
	*/
	if (sizeof(void*) == 4)
		m_PoolLimit = 1l << 31;
	else
		m_PoolLimit = sysconf(_SC_PHYS_PAGES) * m_PageSize;

	m_PoolLimit -= m_CacheSize;

	m_PoolStart = (char*)mmap(NULL, m_PoolLimit, PROT_NONE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (m_PoolStart == MAP_FAILED)
		m_PoolStart = 0;
#endif
	m_PoolPtr = m_PoolStart;
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

void GOrgueMemoryPool::FreePool()
{
#ifdef linux
	if (m_PoolStart)
		munmap(m_PoolStart, m_PoolLimit);
	if (m_CacheSize)
		munmap(m_CacheStart, m_CacheSize);
#endif
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
#ifdef linux
	if (mprotect(m_PoolStart, new_size, PROT_READ | PROT_WRITE) == -1)
		return;
	m_PoolSize = new_size;
#endif	
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

