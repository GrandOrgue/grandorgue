/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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
