/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCacheObject.h"

#include <wx/intl.h>

#include "GOAlloc.h"

void GOCacheObject::InitBeforeLoad() {
  m_IsReady = false;
  m_LoadError.Clear();
}

bool GOCacheObject::InitWithoutExc() {
  InitBeforeLoad();
  try {
    Initialize();
    m_IsReady = true;
  } catch (GOOutOfMemory e) {
    throw e;
  } catch (wxString error) {
    m_LoadError = error;
  } catch (const std::exception &e) {
    m_LoadError = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = _("Unknown exception");
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
    m_LoadError = error;
  } catch (const std::exception &e) {
    m_LoadError = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = _("Unknown exception");
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
    m_LoadError = error;
  } catch (const std::exception &e) {
    m_LoadError = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    m_LoadError = _("Unknown exception");
  }
  return m_IsReady;
}
