/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueCacheWriter.h"

#include "GrandOrgueDef.h"
#include <wx/zstream.h>

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

bool GOrgueCacheWriter::WriteHeader()
{
	int magic = GRANDORGUE_CACHE_MAGIC;
	if (!Write(&magic, sizeof(magic)))
		return false;
	return true;
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
