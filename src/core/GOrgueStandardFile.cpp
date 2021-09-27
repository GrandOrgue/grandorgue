/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueStandardFile.h"

GOrgueStandardFile::GOrgueStandardFile(const wxString& path) :
	m_Path(path),
	m_Name(path),
	m_Size(0)
{
}

GOrgueStandardFile::GOrgueStandardFile(const wxString& path, const wxString& name) :
	m_Path(path),
	m_Name(name),
	m_Size(0)
{
}

GOrgueStandardFile::~GOrgueStandardFile()
{
	Close();
}

const wxString GOrgueStandardFile::GetName()
{
	return m_Name;
}

const wxString GOrgueStandardFile::GetPath()
{
	return m_Path;
}

size_t GOrgueStandardFile::GetSize()
{
	return m_Size;
}

bool GOrgueStandardFile::Open()
{
	if (!m_File.Open(m_Path, wxFile::read))
		return false;
	m_Size = m_File.Length();
	return true;
}
void GOrgueStandardFile::Close()
{
	m_File.Close();
}

size_t GOrgueStandardFile::Read(void * buffer, size_t len)
{
	ssize_t read = m_File.Read(buffer, len);
	if (read == wxInvalidOffset)
		return 0;
	return read;
}
