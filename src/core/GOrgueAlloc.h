/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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

