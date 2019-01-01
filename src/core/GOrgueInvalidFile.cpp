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

#include "GOrgueInvalidFile.h"

GOrgueInvalidFile::GOrgueInvalidFile(const wxString& name) :
	m_Name(name)
{
}

const wxString GOrgueInvalidFile::GetName()
{
	return m_Name;
}

const wxString GOrgueInvalidFile::GetPath()
{
	return wxEmptyString;
}

bool GOrgueInvalidFile::isValid()
{
	return false;
}

size_t GOrgueInvalidFile::GetSize()
{
	return 0;
}

bool GOrgueInvalidFile::Open()
{
	return false;
}
void GOrgueInvalidFile::Close()
{
}

size_t GOrgueInvalidFile::Read(void * buffer, size_t len)
{
	return 0;
}
