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

#ifndef GORGUEMEMORYPOOL_H_
#define GORGUEMEMORYPOOL_H_

#include <set>
class wxFile;

class GOrgueMemoryPool {
	std::set<void*> m_PoolAllocs;
	char* m_PoolStart;
	char* m_PoolPtr;
	char* m_PoolEnd;
	char* m_CacheStart;
	unsigned long m_PoolSize;
	unsigned long m_PoolLimit;
	unsigned long m_PageSize;
	unsigned long m_CacheSize;
	unsigned long m_MallocSize;
	char m_dummy;

	void InitPool();
	void GrowPool(unsigned long size);
	void FreePool();
	void* PoolAlloc(unsigned length);
	void AddPoolAlloc(void* data);

public:
	GOrgueMemoryPool();
	~GOrgueMemoryPool();

	void *Alloc(unsigned length);
	void *Realloc(void* data, unsigned old_length, unsigned new_length);
	void Free(void* data);

	void *GetCacheData(unsigned long offset, unsigned length);
	bool SetCacheFile(wxFile& cache_file);
	void FreeCacheFile();

	unsigned long AllocSize();
	unsigned long MappedSize();
};


#endif
