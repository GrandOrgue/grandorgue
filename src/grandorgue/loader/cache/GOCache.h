/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCACHE_H_
#define GOCACHE_H_

class GOMemoryPool;
class wxFile;
class wxInputStream;

class GOCache {
  wxInputStream *m_stream;
  wxInputStream *m_fstream;
  wxInputStream *m_zstream;
  GOMemoryPool &m_pool;
  bool m_Mapable;
  bool m_OK;

public:
  GOCache(wxFile &cache_file, GOMemoryPool &pool);
  virtual ~GOCache();

  bool ReadHeader();
  void FreeCacheFile();

  bool Read(void *data, unsigned length);
  /* Allocate and read a block written by WriteBlock */
  void *ReadBlock(unsigned length);

  void Close();
};

#endif
