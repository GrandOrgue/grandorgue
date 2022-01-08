/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef ATOMIC_H
#define ATOMIC_H

#if 1

#include <atomic>

template <class T> class atomic {
private:
  std::atomic<T> m_Value;

public:
  atomic() : m_Value() {}

  atomic(const T &val) : m_Value(val) {}

  atomic(const atomic<T> &val) : m_Value(val.m_Value) {}

  T operator=(const T &val) {
    m_Value = val;
    return val;
  }

  operator T() const { return m_Value; }

  T exchange(const T &val) { return m_Value.exchange(val); }

  bool compare_exchange(T &expected, T new_value) {
    return m_Value.compare_exchange_strong(expected, new_value);
  }

  T fetch_add(const T &value) { return m_Value.fetch_add(value); }
};
#else

#include <wx/thread.h>

template <class T> class atomic {
private:
  wxCriticalSection m_Lock;
  T m_Value;

public:
  atomic() {}

  atomic(const T &val) { m_Value = val; }

  atomic(const atomic<T> &val) { m_Value = val.m_Value; }

  T operator=(const T &val) {
    wxCriticalSectionLocker m_Locker(m_Lock);
    m_Value = val;
    return val;
  }

  operator T() const { return m_Value; }

  T exchange(const T &val) {
    wxCriticalSectionLocker m_Locker(m_Lock);
    T current = m_Value;
    m_Value = val;
    return current;
  }

  bool compare_exchange(T &expected, T new_value) {
    wxCriticalSectionLocker m_Locker(m_Lock);

    if (m_Value == expected) {
      m_Value = new_value;
      return true;
    } else {
      expected = m_Value;
      return false;
    }
  }

  T fetch_add(const T &value) {
    wxCriticalSectionLocker m_Locker(m_Lock);
    T current = m_Value;
    m_Value += value;
    return current;
  }
};
#endif

typedef atomic<unsigned> atomic_uint;
typedef atomic<int> atomic_int;

template <class T> inline T load_once(const T &var) {
  return *(const volatile T *)&var;
}

template <class T> inline void store_once(T &var, const T val) {
  *(const volatile T *)&var = val;
}

#endif
