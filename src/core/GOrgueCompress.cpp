/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueCompress.h"

#include "GOrgueBuffer.h"
#include "GOrgueGZipFormat.h"
#include <wx/mstream.h>
#include <wx/zstream.h>

bool compressBuffer(GOrgueBuffer<uint8_t>& buffer)
{
	wxMemoryOutputStream stream;
	wxZlibOutputStream zstream(stream, -1, wxZLIB_GZIP);
	zstream.Write(buffer.get(), buffer.GetSize());
	if (!zstream.IsOk() || !zstream.Close())
		return false;
	if (!stream.IsOk() || !stream.Close())
		return false;
	GOrgueBuffer<uint8_t> buf(stream.GetLength());
	if (stream.CopyTo(buf.get(), buf.GetSize()) != buf.GetSize())
		return false;
	buffer = std::move(buf);
	return true;
}


bool isBufferCompressed(const GOrgueBuffer<uint8_t>& buffer)
{
	if (buffer.GetSize() < sizeof(GOGZipHeader) + sizeof(GOGZipTrailer))
		return false;
	const GOGZipHeader* header = reinterpret_cast<const GOGZipHeader*>(buffer.get());
	if (header->signature == GZIP_SIGNATURE)
		return true;
	return false;
}

bool uncompressBuffer(GOrgueBuffer<uint8_t>& buffer)
{
	wxMemoryOutputStream mstream;
	{
		wxMemoryInputStream stream(buffer.get(), buffer.GetSize());
		wxZlibInputStream zstream(stream, wxZLIB_GZIP);
		zstream.Read(mstream);
		if (!zstream.Eof())
			return false;
	}
	if (!mstream.IsOk() || !mstream.Close())
		return false;
	GOrgueBuffer<uint8_t> buf(mstream.GetLength());
	if (mstream.CopyTo(buf.get(), buf.GetSize()) != buf.GetSize())
		return false;
	buffer = std::move(buf);
	return true;
}
