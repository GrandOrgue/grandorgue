/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueHash.h"

#include <wx/utils.h>

GOrgueHash::GOrgueHash() :
	m_Hash(),
	m_Done(false)
{
	SHA1_Init(&m_ctx);
}

GOrgueHash::~GOrgueHash()
{
}

const GOrgueHashType& GOrgueHash::getHash()
{
	if (!m_Done)
	{
		SHA1_Final(m_Hash.hash, &m_ctx);
		m_Done = true;
	}
	return m_Hash;
}

wxString GOrgueHash::getStringHash()
{
	getHash();
	wxString result = wxEmptyString;
	for(unsigned i = 0; i < 20; i++)
		result += wxDecToHex(m_Hash.hash[i]);
	return result;
}

void GOrgueHash::Update(const void* data, unsigned len)
{
	SHA1_Update(&m_ctx, data, len);
}

void GOrgueHash::Update(const wxString& str)
{
	Update((const wxChar*)str.c_str(), (str.Length() + 1) * sizeof(wxChar));
}

void GOrgueHash::Update(signed int value)
{
	Update(&value, sizeof(value));
}

void GOrgueHash::Update(unsigned int value)
{
	Update(&value, sizeof(value));
}

void GOrgueHash::Update(signed long value)
{
	Update(&value, sizeof(value));
}

void GOrgueHash::Update(unsigned long value)
{
	Update(&value, sizeof(value));
}

void GOrgueHash::Update(signed long long value)
{
	Update(&value, sizeof(value));
}

void GOrgueHash::Update(unsigned long long value)
{
	Update(&value, sizeof(value));
}
