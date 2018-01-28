/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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
