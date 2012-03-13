/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include <wx/wx.h>
#include <wx/file.h>
#ifdef linux
#include <sys/mman.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif
#include <errno.h>

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
	m_MallocSize(0),
	m_AllocError(0),
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
	m_MallocSize += length;
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
	m_MallocSize = m_MallocSize + new_length - old_length;
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
		m_AllocError = 0;
		return data;
	}
	GrowPool(length);
	if (m_PoolPtr + length < m_PoolEnd)
	{
		void* data = m_PoolPtr;
		m_PoolPtr += length;
		m_AllocError = 0;
		return data;
	}
	if (!m_AllocError++)
	{
		wxLogError(wxT("PoolAlloc failed: %d %d %08x %08x %08x"),
			   length, m_PoolSize, m_PoolStart, m_PoolPtr, m_PoolEnd);
	}
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

unsigned long GOrgueMemoryPool::GetAllocSize()
{
	return m_PoolSize + m_MallocSize;
}

unsigned long GOrgueMemoryPool::GetMappedSize()
{
	return m_CacheSize;
}

unsigned long GOrgueMemoryPool::GetPoolSize()
{
	return m_PoolLimit;
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
		wxLogError(_("Memory mapping of the cache file failed with error code %d"), errno);
	}
	else
		result = true;
	
#endif
#ifdef __WIN32__
	int last_error = 0;
	HANDLE map = CreateFileMapping((HANDLE)_get_osfhandle(cache_file.fd()), NULL, PAGE_READONLY, 0, 0, NULL);
	last_error = GetLastError();
	if (map)
	{
		m_CacheSize = cache_file.Length();
		m_CacheStart = (char*)MapViewOfFile(map, FILE_MAP_READ, 0, 0, m_CacheSize);
		last_error = GetLastError();
		if (!m_CacheStart)
			m_CacheSize = 0;
		CloseHandle(map);
	}
	if (!m_CacheStart)
		wxLogError(_("Memory mapping of the cache file failed with error code %d"), last_error);
#endif

	InitPool();
	return result;
}

void GOrgueMemoryPool::CalculatePageSize()
{
	m_PageSize = 4096;
#ifdef linux
	m_PageSize = sysconf(_SC_PAGESIZE);
#endif
#ifdef __WIN32__
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_PageSize = info.dwPageSize;
#endif
}

void GOrgueMemoryPool::CalculatePoolLimit()
{
	m_PoolLimit = 0;
#ifdef linux
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
#endif
#ifdef __WIN32__
	SYSTEM_INFO info;
	MEMORY_BASIC_INFORMATION mem_info;
	MEMORYSTATUSEX mem_stat;
	GetSystemInfo(&info);

	/* Search for largest block */
	m_PoolLimit = 0;
	for(char* ptr = 0; ptr < info.lpMaximumApplicationAddress; ptr += mem_info.RegionSize)
	{
		if (VirtualQuery(ptr, &mem_info, sizeof(mem_info)) <= 0)
		    break;
		if (mem_info.State == MEM_FREE)
			if (m_PoolLimit < mem_info.RegionSize)
				m_PoolLimit = mem_info.RegionSize;
	}
	/* Limit with the available system memory */
	if (GlobalMemoryStatusEx(&mem_stat))
		if (m_PoolLimit + m_CacheSize > mem_stat.ullTotalPhys)
		{
			if (mem_stat.ullTotalPhys > m_CacheSize)
				m_PoolLimit = mem_stat.ullTotalPhys - m_CacheSize;
			else
				m_PoolLimit = 0;
		}
#endif
}

bool GOrgueMemoryPool::AllocatePool()
{
#ifdef linux
	m_PoolStart = (char*)mmap(NULL, m_PoolLimit, PROT_NONE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (m_PoolStart == MAP_FAILED)
	{
		m_PoolStart = 0;
		return false;
	}
#endif
#ifdef __WIN32__
	m_PoolStart = (char*)VirtualAlloc(NULL, m_PoolLimit, MEM_RESERVE, PAGE_NOACCESS);
	if (!m_PoolStart)
		return false;
#endif
	return true;
}

void GOrgueMemoryPool::InitPool()
{
	m_AllocError = 0;
	m_PoolStart = 0;
	m_PoolSize = 0;
	CalculatePageSize();
	CalculatePoolLimit();
	wxLogDebug(wxT("Memory pool limit: %ld bytes (page size: %d)"), m_PoolLimit, m_PageSize);

	while (m_PoolLimit)
	{
		if (AllocatePool())
			break;
		if (m_PoolLimit < 500 * 1024 * 1024)
		{
			wxLogWarning(wxT("Initialization of the memory pool failed (size: %ld bytes)"), m_PoolLimit);
			m_PoolLimit = 0;
			break;
		}
		m_PoolLimit -= 1000 * m_PageSize;
	}
	m_PoolPtr = m_PoolStart;
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

void GOrgueMemoryPool::FreePool()
{
	if (m_PoolAllocs.size())
	{
		wxLogError(wxT("Freeing non-empty memory pool"));
	}
#ifdef linux
	if (m_PoolStart)
		munmap(m_PoolStart, m_PoolLimit);
	if (m_CacheSize)
		munmap(m_CacheStart, m_CacheSize);
#endif
#ifdef __WIN32__
	if (m_PoolStart)
		VirtualFree(m_PoolStart, 0, MEM_RELEASE);
	if (m_CacheSize)
		UnmapViewOfFile(m_CacheStart);
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
#ifdef __WIN32__
	if (!VirtualAlloc(m_PoolStart, new_size, MEM_COMMIT, PAGE_READWRITE))
		return;
	m_PoolSize = new_size;
#endif
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

