/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
