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

#ifndef GORGUEFILE_H
#define GORGUEFILE_H

#include <wx/string.h>

template<class T>
class GOrgueBuffer;

class GOrgueFile
{
public:
	virtual ~GOrgueFile()
	{
	}

	virtual size_t GetSize() = 0;
	virtual const wxString GetName() = 0;
	virtual const wxString GetPath() = 0;

	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual size_t Read(void * buffer, size_t len) = 0;

	template<class T>
	bool Read(GOrgueBuffer<T>& buf)
	{
		return Read(buf.get(), buf.GetSize()) == buf.GetSize();
	}

	template<class T>
	bool ReadContent(GOrgueBuffer<T>& buf)
	{
		if (!Open())
			return false;
		if (GetSize() % sizeof(T))
		{
			Close();
			return false;
		}
		buf.resize(GetSize() / sizeof(T));
		bool ret = Read(buf);
		Close();
		return ret;
	}

	virtual bool isValid()
	{
		return true;
	}
};

#endif
