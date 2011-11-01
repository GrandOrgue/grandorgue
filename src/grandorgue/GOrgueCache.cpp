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

#include "GOrgueCache.h"
#include "GOrgueMemoryPool.h"
#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>

GOrgueCache::GOrgueCache(wxFile& cache_file, GOrgueMemoryPool& pool) :
	m_stream(0),
	m_fstream(0),
	m_zstream(0),
	m_pool(pool),
	m_Mapable(false)
{
	m_fstream = new wxFileInputStream(cache_file);
	m_zstream = new wxZlibInputStream(*m_fstream);
	m_zstream->Peek();
	if (m_fstream->IsOk() && m_zstream->IsOk())
	{
		/* It looks like compressed data */
		m_stream = m_zstream;
		m_Mapable = false;
	}
	else
	{
		delete m_zstream;
		delete m_fstream;
		m_zstream = 0;
		m_fstream = new wxFileInputStream(cache_file);
		m_stream = m_fstream;
		m_Mapable = true;
	}
	m_stream->SeekI(0, wxFromStart);

	if (m_stream->TellI() == wxInvalidOffset)
		m_Mapable = false;
	if (m_Mapable)
		m_Mapable = m_pool.SetCacheFile(cache_file);
}

GOrgueCache::~GOrgueCache()
{
	Close();
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
	void* data = m_pool.Alloc(length);
	if (data == NULL)
		throw (wxString)_("< out of memory allocating samples");

	m_stream->Read(data, length);
	if (m_stream->LastRead() != length)
	{
		m_pool.Free(data);
		return NULL;
	}
	return data;
}
