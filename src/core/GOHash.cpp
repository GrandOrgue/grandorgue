/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOHash.h"

#include <wx/utils.h>

GOHash::GOHash() : m_Hash(), m_Done(false) { SHA1_Init(&m_ctx); }

GOHash::~GOHash() {}

const GOHashType &GOHash::getHash() {
  if (!m_Done) {
    SHA1_Final(m_Hash.hash, &m_ctx);
    m_Done = true;
  }
  return m_Hash;
}

wxString GOHash::getStringHash() {
  getHash();
  wxString result = wxEmptyString;
  for (unsigned i = 0; i < 20; i++)
    result += wxDecToHex(m_Hash.hash[i]);
  return result;
}

void GOHash::Update(const void *data, unsigned len) {
  SHA1_Update(&m_ctx, data, len);
}

void GOHash::Update(const wxString &str) {
  Update((const wxChar *)str.c_str(), (str.Length() + 1) * sizeof(wxChar));
}

void GOHash::Update(signed int value) { Update(&value, sizeof(value)); }

void GOHash::Update(unsigned int value) { Update(&value, sizeof(value)); }

void GOHash::Update(signed long value) { Update(&value, sizeof(value)); }

void GOHash::Update(unsigned long value) { Update(&value, sizeof(value)); }

void GOHash::Update(signed long long value) { Update(&value, sizeof(value)); }

void GOHash::Update(unsigned long long value) { Update(&value, sizeof(value)); }
