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

GOrgueCache::GOrgueCache(wxInputStream& stream, GOrgueMemoryPool& pool) :
	m_stream(stream),
	m_pool(pool)
{
}

bool GOrgueCache::Read(void* data, unsigned length)
{
	m_stream.Read(data, length);
	if (m_stream.LastRead() != length)
		return false;
	return true;
}

void* GOrgueCache::ReadBlock(unsigned length)
{
	void* data = m_pool.Alloc(length);
	if (data == NULL)
		throw (wxString)_("< out of memory allocating samples");

	m_stream.Read(data, length);
	if (m_stream.LastRead() != length)
	{
		m_pool.Free(data);
		return NULL;
	}
	return data;
}
