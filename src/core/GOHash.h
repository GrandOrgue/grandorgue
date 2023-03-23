/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOHASH_H
#define GOHASH_H

#include <cstdint>
#include <wx/string.h>

#include "contrib/sha1.h"

typedef struct _GOHashType {
  uint8_t hash[20];
} GOHashType;

class GOHash {
private:
  SHA_CTX m_ctx;
  GOHashType m_Hash;
  bool m_Done;

public:
  GOHash();
  ~GOHash();

  const GOHashType &getHash();
  wxString getStringHash();

  void Update(const void *data, unsigned len);
  void Update(const wxString &str);
  void Update(signed int value);
  void Update(unsigned int value);
  void Update(signed long value);
  void Update(unsigned long value);
  void Update(signed long long value);
  void Update(unsigned long long value);
};

#endif
