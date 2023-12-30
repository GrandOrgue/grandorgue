/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCache.h"

#include <wx/wfstream.h>
#include <wx/zstream.h>

#include "GOAlloc.h"
#include "GOMemoryPool.h"
#include "go_defs.h"

GOCache::GOCache(wxFile &cache_file, GOMemoryPool &pool)
  : m_stream(0),
    m_fstream(0),
    m_zstream(0),
    m_pool(pool),
    m_Mapable(false),
    m_OK(false) {
  int magic;

  m_stream = m_fstream = new wxFileInputStream(cache_file);

  m_fstream->Read(&magic, sizeof(magic));
  if (
    m_fstream->LastRead() == sizeof(magic) && magic == GRANDORGUE_CACHE_MAGIC) {
    m_Mapable = true;
    m_OK = true;
  } else {
    m_fstream->SeekI(0, wxFromStart);
    m_stream = m_zstream = new wxZlibInputStream(*m_fstream);
    if (m_fstream->IsOk() && m_zstream->IsOk()) {
      m_zstream->Read(&magic, sizeof(magic));
      if (
        m_zstream->LastRead() == sizeof(magic)
        && magic == GRANDORGUE_CACHE_MAGIC) {
        m_Mapable = false;
        m_OK = true;
      }
    }
  }

  if (!m_OK || m_stream->TellI() == wxInvalidOffset)
    m_Mapable = false;
  if (m_Mapable)
    m_Mapable = m_pool.SetCacheFile(cache_file);
}

GOCache::~GOCache() { Close(); }

bool GOCache::ReadHeader() { return m_OK; }

void GOCache::Close() {
  if (m_zstream)
    delete m_zstream;
  m_zstream = 0;
  if (m_fstream)
    delete m_fstream;
  m_fstream = 0;
}

bool GOCache::Read(void *data, unsigned length) {
  m_stream->Read(data, length);
  if (m_stream->LastRead() != length)
    return false;
  return true;
}

void GOCache::FreeCacheFile() {
  m_Mapable = false;
  m_pool.FreeCacheFile();
}

void *GOCache::ReadBlock(unsigned length) {
  if (m_Mapable) {
    void *data = m_pool.GetCacheData(m_stream->TellI(), length);
    if (data) {
      m_stream->SeekI(length, wxFromCurrent);
      return data;
    }
  }
  void *data = m_pool.Alloc(length, true);
  if (data == NULL)
    throw GOOutOfMemory();

  m_stream->Read(data, length);
  if (m_stream->LastRead() != length) {
    m_pool.Free(data);
    return NULL;
  }
  return data;
}
