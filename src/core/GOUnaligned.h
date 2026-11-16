/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOUNALIGNED_H
#define GOUNALIGNED_H

#include <array>
#include <bit>
#include <type_traits>

/**
 * Holds a value of a trivially copyable type T in an array of bytes, so that
 * it can be stored at any address, for example in the samples at arbitrary
 * byte offsets of the memory pool or of a cache file. A pointer to T would
 * promise an alignment that such memory does not have.
 * @tparam T the type of the value
 */
template <typename T> class GOUnaligned {
private:
  static_assert(std::is_trivially_copyable_v<T>);

  using Bytes = std::array<unsigned char, sizeof(T)>;

  Bytes m_Bytes;

public:
  GOUnaligned(T val = T()) : m_Bytes(std::bit_cast<Bytes>(val)) {}

  operator T() const { return std::bit_cast<T>(m_Bytes); }

  GOUnaligned &operator=(const T &val) {
    m_Bytes = std::bit_cast<Bytes>(val);
    return *this;
  }
};

#endif /* GOUNALIGNED_H */
