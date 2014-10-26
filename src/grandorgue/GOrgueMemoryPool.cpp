/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueMemoryPool.h"

#include <wx/file.h>
#include <wx/intl.h>
#include <wx/utils.h>
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif
#ifdef __WXMAC__
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#endif
#include <errno.h>

static inline void touchMemory(const char* pos)
{
	*(const volatile char*)pos;
}

GOrgueMemoryPool::GOrgueMemoryPool() :
	wxThread(wxTHREAD_JOINABLE),
	m_PoolStart(0),
	m_PoolPtr(0),
	m_PoolEnd(0),
	m_CacheStart(0),
	m_PoolSize(0),
	m_PoolLimit(0),
	m_PageSize(4096),
	m_CacheSize(0),
	m_MallocSize(0),
	m_MemoryLimit(0),
	m_AllocError(0)
{
	Create();
	InitPool();
}

GOrgueMemoryPool::~GOrgueMemoryPool()
{
	if (IsAlive())
		Delete();
	FreePool();
}

void *GOrgueMemoryPool::Alloc(size_t length, bool final)
{
	if (m_MemoryLimit && m_CacheSize + m_PoolSize + m_MallocSize > m_MemoryLimit)
		return NULL;
	if (!final)
		return malloc(length);
	GOMutexLocker locker(m_mutex);
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
	GOMutexLocker locker(m_mutex);
	if (m_PoolAllocs.count(data))
	{
		/* Pool memory is not individually freed */
		m_PoolAllocs.erase(m_PoolAllocs.find(data));
		return;
	}
	free(data);
}

void *GOrgueMemoryPool::MoveToPool(void* data, size_t length)
{
	if (m_PoolAllocs.count(data))
	{
		wxLogWarning(_("Element already in the pool"));
		return data;
	}
	void* new_data = Alloc(length, true);
	if (!new_data)
	{
		Free(data);
		return NULL;
	}
	memcpy(new_data, data, length);
	Free(data);
	return new_data;
}

void GOrgueMemoryPool::AddPoolAlloc(void* data)
{
	m_PoolAllocs.insert(data);
}

void* GOrgueMemoryPool::PoolAlloc(size_t length)
{
	char* new_ptr;

	if (!m_PoolStart)
		return NULL;
	if (!length)
		length++;

	new_ptr = m_PoolPtr + length;
	if (m_PoolPtr <= new_ptr && new_ptr < m_PoolEnd)
	{
		void* data = m_PoolPtr;
		m_PoolPtr += length;
		m_AllocError = 0;
		return data;
	}

	GrowPool(length);

	new_ptr = m_PoolPtr + length;
	if (m_PoolPtr <= new_ptr && new_ptr < m_PoolEnd)
	{
		void* data = m_PoolPtr;
		m_PoolPtr += length;
		m_AllocError = 0;
		return data;
	}
	if (!m_AllocError++)
	{
		if (m_PoolSize + m_PageSize < m_PoolLimit)
		{
			wxLogError(wxT("PoolAlloc failed: %d %llu %llu %llu %p %p %p"),
				   length, (unsigned long long)m_PoolSize, (unsigned long long)m_PoolLimit, 
				   (unsigned long long)m_PoolIncrement, m_PoolStart, m_PoolPtr, m_PoolEnd);
		}
		else
		{
			#if !defined (_WIN32) || defined (_WIN64)
			wxLogWarning(_("Memory pool is full: %llu of %llu used"), 
				     (unsigned long long)m_PoolSize, (unsigned long long)m_PoolLimit);
			#endif
		}
	}
	return NULL;
}

void *GOrgueMemoryPool::GetCacheData(size_t offset, size_t length)
{
	if (!length)
		return NULL;
	if (m_CacheStart)
	{
		char* data = m_CacheStart + offset;
		for (unsigned i = 0; i < length; i+= m_PageSize)
			touchMemory(data + i);
		if (length)
			touchMemory(data + length - 1);
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

size_t GOrgueMemoryPool::GetAllocSize()
{
	return m_PoolSize + m_MallocSize;
}

size_t GOrgueMemoryPool::GetMappedSize()
{
	return m_CacheSize;
}

size_t GOrgueMemoryPool::GetPoolSize()
{
	return m_PoolLimit;
}

size_t GOrgueMemoryPool::GetPoolUsage()
{
	return m_PoolSize;
}

size_t GOrgueMemoryPool::GetMemoryLimit()
{
	return m_MemoryLimit;
}

bool GOrgueMemoryPool::IsPoolFull()
{
	return m_AllocError > 0;
}

void GOrgueMemoryPool::SetMemoryLimit(size_t limit)
{
	m_MemoryLimit = limit;
}

bool GOrgueMemoryPool::SetCacheFile(wxFile& cache_file)
{
	bool result = false;
	FreePool();

#if defined __linux__ || __WXMAC__
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
	else
		result = true;
#endif

	InitPool();
	return result;
}

size_t GOrgueMemoryPool::GetPageSize()
{
#ifdef __linux__
	return sysconf(_SC_PAGESIZE);
#endif
#ifdef __WXMAC__
	return getpagesize();
#endif
#ifdef __WIN32__
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwPageSize;
#endif
	return 4096;
}

size_t GOrgueMemoryPool::GetSystemMemory()
{
#ifdef __linux__
	return sysconf(_SC_PHYS_PAGES) * GetPageSize();
#endif
#ifdef __WXMAC__
	int mib[2];
	int64_t mem;
	size_t l;
	
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	l = sizeof(mem);
	sysctl(mib, 2, &mem, &l, NULL, 0);

	return mem;
#endif
#ifdef __WIN32__
	MEMORYSTATUSEX mem_stat;
	mem_stat.dwLength = sizeof(mem_stat);

	if (GlobalMemoryStatusEx(&mem_stat))
	{
		return mem_stat.ullTotalPhys;
	}
	
#endif
	return 0;
}

size_t GOrgueMemoryPool::GetSystemMemoryLimit()
{
	size_t mem = GetSystemMemory() / (1024 * 1024);
	if (mem > 200)
		mem -= 200;
	return mem;
}

size_t GOrgueMemoryPool::GetVMALimit()
{
#ifdef __linux__
	/* On 32 bit, limit to 2.5 GB address space (so 500 MB address space is left for the rest of GO) */
	if (sizeof(void*) == 4)
		return (1l << 31) + (1l << 29);
	else
		return SIZE_MAX;
#endif
#ifdef __WXMAC__
	if (sizeof(void*) == 4)
		return (1ul << 31) + (1ul << 29);
	else
		return SIZE_MAX;
#endif
#ifdef __WIN32__
	SYSTEM_INFO info;
	MEMORY_BASIC_INFORMATION mem_info;
	GetSystemInfo(&info);

	/* Search for largest block */
	size_t max_block = 0;
	for(char* ptr = 0; ptr < info.lpMaximumApplicationAddress; ptr += mem_info.RegionSize)
	{
		if (VirtualQuery(ptr, &mem_info, sizeof(mem_info)) <= 0)
		    break;
		if (mem_info.State == MEM_FREE)
			if (max_block < mem_info.RegionSize)
				max_block = mem_info.RegionSize;
	}
	return max_block;
#endif
	return 0;
}

void GOrgueMemoryPool::CalculatePoolLimit()
{
	size_t vma = GetVMALimit();
	size_t memory = GetSystemMemory();
	if (memory > m_CacheSize)
		memory -= m_CacheSize;
	else
		memory = 0;
	m_PoolLimit = std::min (memory, vma);
	m_PoolIncrement = 1000 * m_PageSize;
}

bool GOrgueMemoryPool::AllocatePool()
{
#if defined __linux__ || __WXMAC__
	m_PoolStart = (char*)mmap(NULL, m_PoolLimit, PROT_NONE, MAP_SHARED|MAP_ANON, -1, 0);
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
	m_PageSize = GetPageSize();
	CalculatePoolLimit();
	wxLogDebug(wxT("Memory pool limit: %llu bytes (page size: %d)"), (unsigned long long)m_PoolLimit, (int)m_PageSize);

	while (m_PoolLimit)
	{
		if (AllocatePool())
			break;
		if (m_PoolLimit < 500 * 1024 * 1024)
		{
			wxLogWarning(wxT("Initialization of the memory pool failed (size: %llu bytes)"), (unsigned long long)m_PoolLimit);
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
#if defined __linux__ || __WXMAC__
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

void GOrgueMemoryPool::GrowPool(size_t length)
{
	size_t new_size = m_PoolSize + m_PoolIncrement;
	while (new_size < m_PoolSize + length)
		new_size += m_PageSize;
	if (new_size > m_PoolLimit || new_size < m_PoolSize)
		new_size = m_PoolLimit;
	if (m_PoolSize >= m_PoolLimit)
		return;
#if defined __linux__ || __WXMAC__
	if (mprotect(m_PoolStart, new_size, PROT_READ | PROT_WRITE) == -1)
		return;
	m_PoolSize = new_size;
#endif	
#ifdef __WIN32__
	if (!VirtualAlloc(m_PoolStart + m_PoolSize, new_size - m_PoolSize, MEM_COMMIT, PAGE_READWRITE))
		return;
	m_PoolSize = new_size;
#endif
	m_PoolEnd = m_PoolStart + m_PoolSize;
}

void GOrgueMemoryPool::StartThread()
{
	Run();
}

void* GOrgueMemoryPool::Entry()
{
	while (!TestDestroy())
	{
		for(size_t pos = 0, i = 0; pos < m_CacheSize; pos+= m_PageSize, i++)
		{
			touchMemory(m_CacheStart + pos);
			if ((i % 256) == 0)
			{
				if (TestDestroy())
					return NULL;
				wxMilliSleep(50);
			}
		}
		for(size_t pos = 0, i = 0; pos < m_PoolSize; pos+= m_PageSize, i++)
		{
			touchMemory(m_PoolStart + pos);
			if ((i % 256) == 0)
			{
				if (TestDestroy())
					return NULL;
				wxMilliSleep(50);
			}
		}
	}
	return NULL;
}
