/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCacheObject.h"

#include <wx/intl.h>

#include "GOAlloc.h"

void GOCacheObject::SetGroupAndPrefix(
  const wxString &group, const wxString &keyPrefix) {
  m_group = group;
  m_KeyPrefix = keyPrefix;
}

void GOCacheObject::InitBeforeLoad() {
  m_IsReady = false;
  m_LoadError.Clear();
}

const wxString GOCacheObject::GenerateMessage(const wxString &srcMsg) const {
  wxString res;

  if (!m_group.IsEmpty()) {
    res << m_group;
    if (!m_KeyPrefix.IsEmpty()) {
      res << "/";
      res << m_KeyPrefix;
    }
    res << ": ";
  }
  res << srcMsg;
  return res;
}

bool GOCacheObject::InitWithoutExc() {
  InitBeforeLoad();
  try {
    Initialize();
    m_IsReady = true;
  } catch (GOOutOfMemory e) {
    throw e;
  } catch (wxString error) {
    m_LoadError = GenerateMessage(error);
  } catch (const std::exception &e) {
    m_LoadError = GenerateMessage(e.what());
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = GenerateMessage(_("Unknown exception"));
  }
  return m_IsReady;
}

bool GOCacheObject::LoadFromFileWithoutExc(
  const GOFileStore &fileStore, GOMemoryPool &pool) {
  InitBeforeLoad();
  try {
    LoadData(fileStore, pool);
    m_IsReady = true;
  } catch (GOOutOfMemory e) {
    throw e;
  } catch (wxString error) {
    m_LoadError = GenerateMessage(error);
  } catch (const std::exception &e) {
    m_LoadError = GenerateMessage(e.what());
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = GenerateMessage(_("Unknown exception"));
  }
  return m_IsReady;
}

bool GOCacheObject::LoadFromCacheWithoutExc(
  GOMemoryPool &pool, GOCache &cache) {
  InitBeforeLoad();
  try {
    m_IsReady = LoadCache(pool, cache);
  } catch (GOOutOfMemory e) {
    throw e;
  } catch (wxString error) {
    m_LoadError = GenerateMessage(error);
  } catch (const std::exception &e) {
    m_LoadError = GenerateMessage(e.what());
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = GenerateMessage(_("Unknown exception"));
  }
  return m_IsReady;
}
