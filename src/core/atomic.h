/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef ATOMIC_H
#define ATOMIC_H

#if 1

#include <atomic>

template<class T>
class atomic
{
private:
	std::atomic<T> m_Value;

public:
	atomic() :
		m_Value()
	{
	}

	atomic(const T& val) :
		m_Value(val)
	{
	}

	atomic(const atomic<T>& val) :
		m_Value(val.m_Value)
	{
	}

	T operator=(const T& val)
	{
		m_Value = val;
		return val;
	}

	operator T() const
	{
		return m_Value;
	}

	T exchange(const T& val)
	{
		return m_Value.exchange(val);
	}

	bool compare_exchange(T& expected, T new_value)
	{
		return m_Value.compare_exchange_strong(expected, new_value);
	}

	T fetch_add(const T& value)
	{
		return m_Value.fetch_add(value);
	}
};
#else

#include <wx/thread.h>

template<class T>
class atomic
{
private:
	wxCriticalSection m_Lock;
	T m_Value;

public:
	atomic()
	{
	}

	atomic(const T& val)
	{
		m_Value = val;
	}

	atomic(const atomic<T>& val)
	{
		m_Value = val.m_Value;
	}

	T operator=(const T& val)
	{
		wxCriticalSectionLocker m_Locker(m_Lock);
		m_Value = val;
		return val;
	}

	operator T() const
	{
		return m_Value;
	}

	T exchange(const T& val)
	{
		wxCriticalSectionLocker m_Locker(m_Lock);
		T current = m_Value;
		m_Value = val;
		return current;
	}

	bool compare_exchange(T& expected, T new_value)
	{
		wxCriticalSectionLocker m_Locker(m_Lock);

		if (m_Value == expected)
		{
			m_Value = new_value;
			return true;
		}
		else
		{
			expected = m_Value;
			return false;
		}
	}

	T fetch_add(const T& value)
	{
		wxCriticalSectionLocker m_Locker(m_Lock);
		T current = m_Value;
		m_Value += value;
		return current;
	}
};
#endif

typedef atomic<unsigned> atomic_uint;
typedef atomic<int> atomic_int;

#endif
