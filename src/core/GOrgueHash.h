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

#ifndef GORGUEHASH_H
#define GORGUEHASH_H

#include "contrib/sha1.h"
#include <wx/string.h>

typedef struct _GOrgueHashType
{
	uint8_t hash[20];
} GOrgueHashType;

class GOrgueHash
{
private:
	SHA_CTX m_ctx;
	GOrgueHashType m_Hash;
	bool m_Done;

public:
	GOrgueHash();
	~GOrgueHash();

	const GOrgueHashType& getHash();
	wxString getStringHash();

	void Update(const void* data, unsigned len);
	void Update(const wxString& str);
	void Update(signed int value);
	void Update(unsigned int value);
	void Update(signed long value);
	void Update(unsigned long value);
	void Update(signed long long value);
	void Update(unsigned long long value);
};

#endif
