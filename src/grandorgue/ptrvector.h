/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>

template<class T>
class ptr_vector : protected std::vector<T*> {
private: 
	/* Disable copying as it will break things */
	ptr_vector(const ptr_vector&);
	const ptr_vector operator=(const ptr_vector&);

public:
	ptr_vector(unsigned new_size = 0) : 
		std::vector<T*>(new_size)
	{
		for (unsigned i = 0; i < new_size; i++)
			at(i) = 0;
	}

	~ptr_vector()
	{
		for (unsigned i = 0; i < size(); i++)
			if (at(i))
				delete at(i);
	}

	T*& operator[] (unsigned pos)
	{
		return at(pos);
	}

	const T* operator[] (unsigned pos) const
	{
		return at(pos);
	}

	T*& at (unsigned pos)
	{
		return std::vector<T*>::at(pos);
	}

	const T* at (unsigned pos) const
	{
		return std::vector<T*>::at(pos);
	}

	unsigned size() const
	{
		return std::vector<T*>::size();
	}

	void clear()
	{
		resize(0);
	}

	void resize(unsigned new_size)
	{
		unsigned oldsize = size();
		for (unsigned i = new_size; i < oldsize; i++)
			if (at(i))
				delete at(i);
		std::vector<T*>::resize(new_size);
		for (unsigned i = oldsize; i < new_size; i++)
			at(i) = 0;
	}

	void push_back(T* ptr)
	{
		std::vector<T*>::push_back(ptr);
	}
};

#endif
