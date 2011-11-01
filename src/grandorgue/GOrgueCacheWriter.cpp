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

#include <wx/wx.h>
#include <wx/zstream.h>
#include "GOrgueCacheWriter.h"

GOrgueCacheWriter::GOrgueCacheWriter(wxOutputStream& stream, bool compressed) :
	m_zstream(0),
	m_stream(&stream)
{
	if (compressed)
	{
		m_zstream = new wxZlibOutputStream(stream);
		m_stream = m_zstream;
	}
}

GOrgueCacheWriter::~GOrgueCacheWriter()
{
	Close();
}

bool GOrgueCacheWriter::Write(const void* data, unsigned length)
{
	m_stream->Write(data, length);
	if (m_stream->LastWrite() != length)
		return false;
	return true;
}

bool GOrgueCacheWriter::WriteBlock(const void* data, unsigned length)
{
	m_stream->Write(data, length);
	if (m_stream->LastWrite() != length)
		return false;
	return true;
}

void GOrgueCacheWriter::Close()
{
	if (m_stream)
		m_stream->Close();
	if (m_zstream)
		delete m_zstream;
	m_zstream = 0;
	m_stream = 0;
}
