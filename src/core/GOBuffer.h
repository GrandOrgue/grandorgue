/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUFFER_H
#define GOBUFFER_H

#include <string.h>

#include <algorithm>
#include <memory>

#include "GOAlloc.h"

template <class T> class GOBuffer {
private:
  std::unique_ptr<T[]> m_Data;
  size_t m_Count;

public:
  GOBuffer() : m_Data(nullptr), m_Count(0) {}

  GOBuffer(size_t count) : m_Count(count) { m_Data = GOAllocArray<T>(m_Count); }

  GOBuffer(GOBuffer &&b) : m_Data(std::move(b.m_Data)), m_Count(b.m_Count) {
    b.m_Count = 0;
  }

  ~GOBuffer() {}

  size_t GetCount() const { return m_Count; }

  size_t GetSize() const { return m_Count * sizeof(T); }

  T &operator[](unsigned pos) { return m_Data[pos]; }

  const T &operator[](unsigned pos) const { return m_Data[pos]; }

  T *get() { return m_Data.get(); }

  const T *get() const { return m_Data.get(); }

  void free() {
    m_Count = 0;
    m_Data = nullptr;
  }

  void resize(size_t count) {
    std::unique_ptr<T[]> data = GOAllocArray<T>(count);
    if (m_Count && count)
      memcpy(data.get(), m_Data.get(), sizeof(T) * std::min(count, m_Count));
    m_Count = count;
    m_Data = std::move(data);
  }

  GOBuffer &operator=(GOBuffer &&b) {
    m_Data = std::move(b.m_Data);
    m_Count = b.GetCount();
    b.m_Count = 0;
    return *this;
  }

  void Append(const GOBuffer &b) {
    if (!b.GetSize())
      return;
    size_t pos = m_Count;
    resize(pos + b.GetCount());
    memcpy(&m_Data[pos], b.get(), b.GetSize());
  }

  void Append(const T *data, size_t count) {
    if (!count)
      return;
    size_t pos = m_Count;
    resize(pos + count);
    memcpy(&m_Data[pos], data, count * sizeof(T));
  }
};

#endif
