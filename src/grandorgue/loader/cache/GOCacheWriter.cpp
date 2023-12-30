/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCacheWriter.h"

#include <wx/zstream.h>

#include "go_defs.h"

GOCacheWriter::GOCacheWriter(wxOutputStream &stream, bool compressed)
  : m_zstream(0), m_stream(&stream) {
  if (compressed) {
    m_zstream = new wxZlibOutputStream(stream);
    m_stream = m_zstream;
  }
}

GOCacheWriter::~GOCacheWriter() { Close(); }

bool GOCacheWriter::WriteHeader() {
  int magic = GRANDORGUE_CACHE_MAGIC;
  if (!Write(&magic, sizeof(magic)))
    return false;
  return true;
}

bool GOCacheWriter::Write(const void *data, unsigned length) {
  m_stream->Write(data, length);
  if (m_stream->LastWrite() != length)
    return false;
  return true;
}

bool GOCacheWriter::WriteBlock(const void *data, unsigned length) {
  m_stream->Write(data, length);
  if (m_stream->LastWrite() != length)
    return false;
  return true;
}

void GOCacheWriter::Close() {
  if (m_stream)
    m_stream->Close();
  if (m_zstream)
    delete m_zstream;
  m_zstream = 0;
  m_stream = 0;
}
