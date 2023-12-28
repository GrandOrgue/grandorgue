/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCACHEOBJECT_H
#define GOCACHEOBJECT_H

#include <wx/string.h>

class GOCache;
class GOCacheWriter;
class GOFileStore;
class GOHash;
class GOMemoryPool;

class GOCacheObject {
private:
  bool m_IsReady = false;
  wxString m_LoadError;

  void InitBeforeLoad();

protected:
  virtual void Initialize() = 0;
  virtual void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) = 0;
  virtual bool LoadCache(GOMemoryPool &pool, GOCache &cache) = 0;

public:
  virtual ~GOCacheObject() {}

  bool IsReady() const { return m_IsReady; }
  const wxString &GetLoadError() const { return m_LoadError; }

  /**
   * Initialize the object with default values.
   * Catches all exceptions except GOOutOfMemory.
   * Returns whether it was initialised successfully .
   * If no then GetLoadError returns the exception message.
   */
  bool InitWithoutExc();

  /**
   * Load the object from files.
   * Catches all exceptions except GOOutOfMemory.
   * Returns whether it was loaded successfully.
   * If no then GetLoadError returns the exception message.
   */
  bool LoadFromFileWithoutExc(const GOFileStore &fileStore, GOMemoryPool &pool);

  /**
   * Load the object from the cache.
   * Catches all exceptions except GOOutOfMemory.
   * Returns whether it was loaded successfully.
   * If no then GetLoadError returns the exception message.
   */
  bool LoadFromCacheWithoutExc(GOMemoryPool &pool, GOCache &cache);

  virtual bool SaveCache(GOCacheWriter &cache) const = 0;
  virtual void UpdateHash(GOHash &hash) const = 0;
  virtual const wxString &GetLoadTitle() const = 0;
};

#endif
