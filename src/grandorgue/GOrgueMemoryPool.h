/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMEMORYPOOL_H_
#define GORGUEMEMORYPOOL_H_

#include <set>
#include <wx/wx.h>
class wxFile;

class GOrgueOutOfMemory {
};

class GOrgueMemoryPool {
	std::set<void*> m_PoolAllocs;
	char* m_PoolStart;
	char* m_PoolPtr;
	char* m_PoolEnd;
	char* m_CacheStart;
	size_t m_PoolSize;
	size_t m_PoolLimit;
	size_t m_PoolIncrement;
	size_t m_PageSize;
	size_t m_CacheSize;
	size_t m_MallocSize;
	size_t m_MemoryLimit;
	unsigned m_AllocError;
	char m_dummy;

	void InitPool();
	void GrowPool(size_t size);
	void FreePool();
	void* PoolAlloc(size_t length);
	void AddPoolAlloc(void* data);

	void CalculatePageSize();
	void CalculatePoolLimit();
	bool AllocatePool();
public:
	GOrgueMemoryPool();
	~GOrgueMemoryPool();
	void SetMemoryLimit(size_t limit);

	void *Alloc(size_t length);
	void *Realloc(void* data, size_t old_length, size_t new_length);
	void Free(void* data);

	void *GetCacheData(size_t offset, size_t length);
	bool SetCacheFile(wxFile& cache_file);
	void FreeCacheFile();

	size_t GetAllocSize();
	size_t GetMappedSize();
	size_t GetPoolSize();
	size_t GetPoolUsage();
	size_t GetMemoryLimit();
};


#endif
