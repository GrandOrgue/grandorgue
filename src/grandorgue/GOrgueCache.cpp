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

#include "GOrgueCache.h"
#include "GrandOrgueDef.h"
#include "GOrgueMemoryPool.h"
#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>

GOrgueCache::GOrgueCache(wxFile& cache_file, GOrgueMemoryPool& pool) :
	m_stream(0),
	m_fstream(0),
	m_zstream(0),
	m_pool(pool),
	m_Mapable(false),
	m_OK(false)
{
	int magic;

	m_stream = m_fstream = new wxFileInputStream(cache_file);

	m_fstream->Read(&magic, sizeof(magic));
	if (m_fstream->LastRead() == sizeof(magic) &&
	    magic == GRANDORGUE_CACHE_MAGIC)
	{
		m_Mapable = true;
		m_OK = true;
	}
	else
	{
		m_fstream->SeekI(0, wxFromStart);
		m_stream = m_zstream = new wxZlibInputStream(*m_fstream);
		if (m_fstream->IsOk() && m_zstream->IsOk())
		{
			m_zstream->Read(&magic, sizeof(magic));
			if (m_zstream->LastRead() == sizeof(magic) &&
			    magic == GRANDORGUE_CACHE_MAGIC)
			{
				m_Mapable = false;
				m_OK = true;
			}
		}
	}

	if (!m_OK ||  m_stream->TellI() == wxInvalidOffset)
		m_Mapable = false;
	if (m_Mapable)
		m_Mapable = m_pool.SetCacheFile(cache_file);
}

GOrgueCache::~GOrgueCache()
{
	Close();
}

bool GOrgueCache::ReadHeader()
{
	return m_OK;
}


void GOrgueCache::Close()
{
	if (m_zstream)
		delete m_zstream;
	m_zstream = 0;
	if (m_fstream)
		delete m_fstream;
	m_fstream = 0;
}

bool GOrgueCache::Read(void* data, unsigned length)
{
	m_stream->Read(data, length);
	if (m_stream->LastRead() != length)
		return false;
	return true;
}

void GOrgueCache::FreeCacheFile()
{
	m_Mapable = false;
	m_pool.FreeCacheFile();
}

void* GOrgueCache::ReadBlock(unsigned length)
{
	if (m_Mapable)
	{
		void *data = m_pool.GetCacheData(m_stream->TellI(), length);
		if (data)
		{
			m_stream->SeekI(length, wxFromCurrent);
			return data;
		}
	}
	void* data = m_pool.Alloc(length, true);
	if (data == NULL)
		throw GOrgueOutOfMemory();

	m_stream->Read(data, length);
	if (m_stream->LastRead() != length)
	{
		m_pool.Free(data);
		return NULL;
	}
	return data;
}
