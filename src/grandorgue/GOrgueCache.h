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

#ifndef GORGUECACHE_H_
#define GORGUECACHE_H_

class wxInputStream;
class wxFile;
class GOrgueMemoryPool;

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
