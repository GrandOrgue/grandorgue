/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECACHE_H_
#define GORGUECACHE_H_

class GOrgueMemoryPool;
class wxFile;
class wxInputStream;

class GOrgueCache {
	wxInputStream* m_stream;
	wxInputStream* m_fstream;
	wxInputStream* m_zstream;
	GOrgueMemoryPool& m_pool;
	bool m_Mapable;
	bool m_OK;

public:
	GOrgueCache(wxFile& cache_file, GOrgueMemoryPool& pool);
	virtual ~GOrgueCache();

	bool ReadHeader();
	void FreeCacheFile();

	bool Read(void* data, unsigned length);
	/* Allocate and read a block written by WriteBlock */
	void* ReadBlock(unsigned length);

	void Close();
};


#endif
