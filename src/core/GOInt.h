/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOINT_H
#define GOINT_H

#include <stdint.h>
#include <wx/defs.h>

class GOIntHelper {
public:
  static uint8_t swap_value(uint8_t val) { return val; }

  static int8_t swap_value(int8_t val) { return val; }

  static uint16_t swap_value(uint16_t val) { return wxUINT16_SWAP_ALWAYS(val); }

  static int16_t swap_value(int16_t val) { return wxINT16_SWAP_ALWAYS(val); }

  static uint32_t swap_value(uint32_t val) { return wxUINT32_SWAP_ALWAYS(val); }

  static int32_t swap_value(int32_t val) { return wxINT32_SWAP_ALWAYS(val); }

  static uint64_t swap_value(uint64_t val) { return wxUINT64_SWAP_ALWAYS(val); }

  static int64_t swap_value(int64_t val) { return wxINT64_SWAP_ALWAYS(val); }

  static constexpr bool isBigEndian() { return (wxBYTE_ORDER == wxBIG_ENDIAN); }

  static constexpr bool isLittleEndian() { return !isBigEndian(); }
};

#pragma pack(push, 1)

template <typename A, bool enable_swap, A swap_func(A)> class GOInt {
private:
  A m_Value;

  A swap(A val) const {
    if (enable_swap)
      return swap_func(val);
    else
      return val;
  }

  void assign(A val) { m_Value = swap(val); }

public:
  GOInt(A val = 0) { assign(val); }

  operator A() const { return swap(m_Value); }

  A operator=(const A val) {
    assign(val);
    return *this;
  }
};

typedef GOInt<int8_t, false, GOIntHelper::swap_value> GOInt8;
typedef GOInt<uint8_t, false, GOIntHelper::swap_value> GOUInt8;

typedef GOInt<int16_t, false, GOIntHelper::swap_value> GOInt16;
typedef GOInt<int16_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOInt16LE;
typedef GOInt<int16_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOInt16BE;
typedef GOInt<uint16_t, false, GOIntHelper::swap_value> GOUInt16;
typedef GOInt<uint16_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOUInt16LE;
typedef GOInt<uint16_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOUInt16BE;

typedef GOInt<int32_t, false, GOIntHelper::swap_value> GOInt32;
typedef GOInt<int32_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOInt32LE;
typedef GOInt<int32_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOInt32BE;
typedef GOInt<uint32_t, false, GOIntHelper::swap_value> GOUInt32;
typedef GOInt<uint32_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOUInt32LE;
typedef GOInt<uint32_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOUInt32BE;

typedef GOInt<int64_t, false, GOIntHelper::swap_value> GOInt64;
typedef GOInt<int64_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOInt64LE;
typedef GOInt<int64_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOInt64BE;
typedef GOInt<uint64_t, false, GOIntHelper::swap_value> GOUInt64;
typedef GOInt<uint64_t, GOIntHelper::isBigEndian(), GOIntHelper::swap_value>
  GOUInt64LE;
typedef GOInt<uint64_t, GOIntHelper::isLittleEndian(), GOIntHelper::swap_value>
  GOUInt64BE;

class GOInt24 {
private:
  uint16_t lo;
  int8_t hi;

  void assign(int value) {
    lo = (value >> 0) & 0xffff;
    hi = (value >> 16) & 0xff;
  }

public:
  GOInt24(int val = 0) { assign(val); }

  operator int() { return ((hi << 16) | lo); }

  int operator=(const int val) {
    assign(val);
    return *this;
  }
};

class GOInt24LE {
private:
  uint8_t lo, mi;
  int8_t hi;

  void assign(int value) {
    lo = (value >> 0) & 0xff;
    mi = (value >> 8) & 0xff;
    hi = (value >> 16) & 0xff;
  }

public:
  GOInt24LE(int val = 0) { assign(val); }

  operator int() { return ((hi << 16) | (mi << 8) | lo); }

  int operator=(const int val) {
    assign(val);
    return *this;
  }
};

#pragma pack(pop)

#endif
