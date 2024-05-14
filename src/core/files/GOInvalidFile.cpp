/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOInvalidFile.h"

GOInvalidFile::GOInvalidFile(const wxString &name) : m_Name(name) {}

const wxString GOInvalidFile::GetName() { return m_Name; }

const wxString GOInvalidFile::GetPath() { return wxEmptyString; }

bool GOInvalidFile::isValid() { return false; }

size_t GOInvalidFile::GetSize() { return 0; }

bool GOInvalidFile::Open() { return false; }
void GOInvalidFile::Close() {}

size_t GOInvalidFile::Read(void *buffer, size_t len) { return 0; }
