/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCOMPRESS_H_
#define GOSOUNDCOMPRESS_H_

#include <stddef.h>
#include <stdint.h>

#include "GOSoundDefs.h"

static inline int AudioReadCompressed8(const unsigned char *&ptr) {
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

static inline int AudioReadCompressed16(const unsigned char *&ptr) {
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

static inline void AudioWriteCompressed8(
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

static inline void AudioWriteCompressed16(
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

typedef struct {
  unsigned position;
  int value[MAX_OUTPUT_CHANNELS];
  int last[MAX_OUTPUT_CHANNELS];
  int prev[MAX_OUTPUT_CHANNELS];
  const unsigned char *ptr;
} DecompressionCache;

static inline void InitDecompressionCache(DecompressionCache &cache) {
  cache.position = 0;
  cache.ptr = NULL;
  for (unsigned j = 0; j < MAX_OUTPUT_CHANNELS; j++) {
    cache.last[j] = 0;
    cache.value[j] = 0;
    cache.prev[j] = 0;
  }
}

static inline void DecompressionStep(
  DecompressionCache &cache, unsigned channels, bool format16) {
  for (unsigned j = 0; j < channels; j++) {
    int val;
    if (format16)
      val = AudioReadCompressed16(cache.ptr);
    else
      val = AudioReadCompressed8(cache.ptr);
    cache.last[j] = cache.prev[j];
    cache.prev[j] = cache.value[j];
    cache.value[j] = cache.prev[j] + (cache.prev[j] - cache.last[j]) / 2 + val;
  }
  cache.position++;
}

static inline void DecompressTo(
  DecompressionCache &cache,
  unsigned position,
  const unsigned char *data,
  unsigned channels,
  bool format16) {
  if (!cache.ptr || cache.position > position + 1) {
    InitDecompressionCache(cache);
    cache.ptr = data;
  }
  while (cache.position <= position)
    DecompressionStep(cache, channels, format16);
}

#endif
