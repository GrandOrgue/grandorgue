/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueArchiveEntryFile.h"

#include "GOrgueArchive.h"

GOrgueArchiveEntryFile::GOrgueArchiveEntryFile(GOrgueArchive* archive, const wxString& name, size_t offset, size_t len) :
	m_archiv(archive),
	m_Name(name),
	m_Offset(offset),
	m_Length(len),
	m_Pos(0)
{
}

size_t GOrgueArchiveEntryFile::GetSize()
{
	return m_Length;
}

const wxString GOrgueArchiveEntryFile::GetName()
{
	return m_Name;
}

const wxString GOrgueArchiveEntryFile::GetPath()
{
	return wxEmptyString;
}

bool GOrgueArchiveEntryFile::Open()
{
	m_Pos = 0;
	return true;
}

void GOrgueArchiveEntryFile::Close()
{
}

size_t GOrgueArchiveEntryFile::Read(void * buffer, size_t len)
{
	size_t remain = m_Length - m_Pos;
	if (len > remain)
		len = remain;
	len = m_archiv->ReadContent(buffer, m_Offset + m_Pos, len);
	m_Pos += len;
	return len;
}
