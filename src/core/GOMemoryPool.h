/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMEMORYPOOL_H_
#define GOMEMORYPOOL_H_

#include <set>

#include "threading/GOMutex.h"

class wxFile;

class GOMemoryPool {
  GOMutex m_mutex;
  std::set<void *> m_PoolAllocs;
  char *m_PoolStart;
  char *m_PoolPtr;
  char *m_PoolEnd;
  char *m_CacheStart;
  size_t m_PoolSize;
  size_t m_PoolLimit;
  size_t m_PoolIncrement;
  size_t m_PageSize;
  size_t m_CacheSize;
  size_t m_MallocSize;
  size_t m_MemoryLimit;
  unsigned m_AllocError;
  size_t m_TouchPos;
  bool m_TouchCache;

  void InitPool();
  void GrowPool(size_t size);
  void FreePool();
  void *PoolAlloc(size_t length);
  void AddPoolAlloc(void *data);

  static size_t GetVMALimit();
  static size_t GetSystemMemory();
  void CalculatePoolLimit();
  bool AllocatePool();
  bool InMemoryPool(void *ptr);

public:
  GOMemoryPool();
  ~GOMemoryPool();
  void SetMemoryLimit(size_t limit);
  void TouchMemory(std::atomic_bool &stop);

  void *Alloc(size_t length, bool final);
  void *MoveToPool(void *data, size_t length);
  void Free(void *data);

  void *GetCacheData(size_t offset, size_t length);
  bool SetCacheFile(wxFile &cache_file);
  void FreeCacheFile();

  bool IsPoolFull();
  size_t GetAllocSize();
  size_t GetMappedSize();
  size_t GetPoolSize();
  size_t GetPoolUsage();
  size_t GetMemoryLimit();

  static size_t GetSystemMemoryLimit();
  static size_t GetPageSize();
};

#endif
