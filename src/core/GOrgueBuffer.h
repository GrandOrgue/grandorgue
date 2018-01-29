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

#ifndef GORGUEBUFFER_H
#define GORGUEBUFFER_H

#include "GOrgueAlloc.h"
#include <string.h>
#include <memory>

template<class T>
class GOrgueBuffer
{
private:
	std::unique_ptr<T[]> m_Data;
	size_t m_Count;
public:
	GOrgueBuffer():
	m_Data(nullptr),
	m_Count(0)
	{
	}

	GOrgueBuffer(size_t count):
	m_Count(count)
	{
		m_Data = GOrgueAllocArray<T>(m_Count);
	}

	GOrgueBuffer(GOrgueBuffer&& b) :
	m_Data(std::move(b.m_Data)),
	m_Count(b.m_Count)
	{
		b.m_Count = 0;
	}

	~GOrgueBuffer()
	{
	}

	size_t GetCount() const
	{
		return m_Count;
	}

	size_t GetSize() const
	{
		return m_Count * sizeof(T);
	}

	T& operator[] (unsigned pos)
	{
		return m_Data[pos];
	}
	
	const T& operator[] (unsigned pos) const
	{
		return m_Data[pos];
	}

	T* get()
	{
		return m_Data.get();
	}

	const T* get() const
	{
		return m_Data.get();
	}

	void free()
	{
		m_Count = 0;
		m_Data = nullptr;
	}
	
	void resize(size_t count)
	{
		std::unique_ptr<T[]> data = GOrgueAllocArray<T>(count);
		if (m_Count && count)
			memcpy(data.get(), m_Data.get(), sizeof(T) * std::min(count, m_Count));
		m_Count = count;
		m_Data = std::move(data);
	}

	GOrgueBuffer& operator=(GOrgueBuffer&& b)
	{
		m_Data = std::move(b.m_Data);
		m_Count = b.GetCount();
		b.m_Count = 0;
		return *this;
	}

	void Append(const GOrgueBuffer& b)
	{
		if (!b.GetSize())
			return;
		size_t pos = m_Count;
		resize(pos + b.GetCount());
		memcpy(&m_Data[pos], b.get(), b.GetSize());
	}

	void Append(const T* data, size_t count)
	{
		if (!count)
			return;
		size_t pos = m_Count;
		resize(pos + count);
		memcpy(&m_Data[pos], data, count * sizeof(T));
	}
};

#endif
