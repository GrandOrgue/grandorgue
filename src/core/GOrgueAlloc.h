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

#ifndef GORGUEALLOC_H
#define GORGUEALLOC_H

#include <memory>

class GOrgueOutOfMemory {
};

template<class T, class ... Args>
std::unique_ptr<T> GOrgueAlloc(Args... args)
{
	try
	{
		return std::unique_ptr<T>(new T(args...));
	}
	catch(std::bad_alloc& ba)
	{
		throw GOrgueOutOfMemory();
	}
}

template<class T>
std::unique_ptr<T[]> GOrgueAllocArray(size_t count)
{
	try
	{
		return std::unique_ptr<T[]>(new T[count]);
	}
	catch(std::bad_alloc& ba)
	{
		throw GOrgueOutOfMemory();
	}
}

#endif

