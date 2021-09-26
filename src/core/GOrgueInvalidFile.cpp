/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
