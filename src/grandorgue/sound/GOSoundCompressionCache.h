/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCOMPRESSIONCACHE_H
#define GOSOUNDCOMPRESSIONCACHE_H

#include <cstdint>

#include "GOSoundDefs.h"

class GOSoundCompressionCache {
public:
  unsigned m_position;
  int m_value[MAX_OUTPUT_CHANNELS];
  int m_last[MAX_OUTPUT_CHANNELS];
  int m_prev[MAX_OUTPUT_CHANNELS];
  const unsigned char *m_ptr;

  static inline int readCompressed8(const unsigned char *&ptr) {
    int val = *(const int8_t *)ptr;
    if (val & 0x01) {
      val >>= 1;
      ptr++;
    } else if (val & 0x02) {
      /* remaining bits are high byte */
      val = ((val & ~3) << (8 - 2)) | (*(const uint8_t *)(ptr + 1));
      ptr += 2;
    } else if (val & 0x04) {
      val = ((val & ~7) << (16 - 3)) | (*(const uint16_t *)(ptr + 1));
      ptr += 3;
    } else {
      val = ((val & ~7) << (24 - 3)) | ((*(const uint8_t *)(ptr + 1)) << 16)
        | (*(const uint16_t *)(ptr + 2));
      ptr += 4;
    }
    return val;
  }

  static inline int readCompressed16(const unsigned char *&ptr) {
    int val = *(const int16_t *)ptr;
    if (val & 0x01) {
      val >>= 1;
      ptr += 2;
    } else if (val & 0x02) {
      /* remaining bits are high byte */
      val = ((val & ~3) << (8 - 2)) | (*(const uint8_t *)(ptr + 2));
      ptr += 3;
    } else {
      val = ((val & ~3) << (16 - 2)) | (*(const uint16_t *)(ptr + 2));
      ptr += 4;
    }
    return val;
  }

  static inline void writeCompressed8(
    unsigned char *data, unsigned &output_len, int encode) {
    if (-64 <= encode && encode <= 63) {
      *((int8_t *)(data + output_len)) = ((encode) << 1) | 0x01;
      output_len++;
    } else if (-8192 <= encode && encode <= 8191) {
      *((int8_t *)(data + output_len)) = ((encode >> 8) << 2) | 0x02;
      *((uint8_t *)(data + output_len + 1)) = encode & 0xFF;
      output_len += 2;
    } else if (-1048576 <= encode && encode <= 1048575) {
      *((int8_t *)(data + output_len)) = ((encode >> 16) << 3) | 0x04;
      *((uint16_t *)(data + output_len + 1)) = encode & 0xFFFF;
      output_len += 3;
    } else {
      *((int8_t *)(data + output_len)) = ((encode >> 24) << 3) | 0x00;
      *((uint8_t *)(data + output_len + 1)) = (encode >> 16) & 0xFF;
      *((uint16_t *)(data + output_len + 2)) = encode & 0xFFFF;
      output_len += 4;
    }
  }

  static inline void writeCompressed16(
    unsigned char *data, unsigned &output_len, int encode) {
    if (-16384 <= encode && encode <= 16383) {
      *((int16_t *)(data + output_len)) = ((encode) << 1) | 0x01;
      output_len += 2;
    } else if (-2097152 <= encode && encode <= 2097151) {
      *((int16_t *)(data + output_len)) = ((encode >> 8) << 2) | 0x02;
      *((uint8_t *)(data + output_len + 2)) = encode & 0xFF;
      output_len += 3;
    } else {
      *((int16_t *)(data + output_len)) = ((encode >> 16) << 2) | 0x00;
      *((uint16_t *)(data + output_len + 2)) = encode & 0xFFFF;
      output_len += 4;
    }
  }

  inline void Init() {
    m_position = 0;
    m_ptr = nullptr;
    for (unsigned j = 0; j < MAX_OUTPUT_CHANNELS; j++) {
      m_last[j] = 0;
      m_value[j] = 0;
      m_prev[j] = 0;
    }
  }

  inline void DecompressionStep(unsigned channels, bool format16) {
    for (unsigned j = 0; j < channels; j++) {
      int val;
      if (format16)
        val = readCompressed16(m_ptr);
      else
        val = readCompressed8(m_ptr);
      m_last[j] = m_prev[j];
      m_prev[j] = m_value[j];
      m_value[j] = m_prev[j] + (m_prev[j] - m_last[j]) / 2 + val;
    }
    m_position++;
  }

  inline void DecompressTo(
    unsigned position,
    const unsigned char *data,
    unsigned channels,
    bool format16) {
    if (!m_ptr || m_position > position + 1) {
      Init();
      m_ptr = data;
    }
    while (m_position <= position)
      DecompressionStep(channels, format16);
  }
};

#endif
