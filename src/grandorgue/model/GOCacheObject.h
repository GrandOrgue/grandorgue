/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCACHEOBJECT_H
#define GOCACHEOBJECT_H

#include <wx/string.h>

#include "loader/GOLoaderFilename.h"

class GOCache;
class GOCacheWriter;
class GOFileStore;
class GOHash;
class GOMemoryPool;

class GOCacheObject {
private:
  bool m_IsReady = false;

  // the group name in the ODF
  wxString m_group;

  // the ODF key prefix of all settings belong to this object
  wxString m_KeyPrefix;

  wxString m_LoadError;

  void InitBeforeLoad();

protected:
  void SetGroupAndPrefix(const wxString &group, const wxString &keyPrefix);
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

  // Returns the message string prefixed with group and keyPrefix
  const wxString GenerateMessage(const wxString &srcMsg) const;

  static const wxString generateMessage(
    const GOCacheObject *pObjectFor, const wxString &srcMsg) {
    return pObjectFor ? pObjectFor->GenerateMessage(srcMsg) : srcMsg;
  }

  static const wxString generateMessage(
    const GOCacheObject *pObjectFor,
    const GOLoaderFilename *pFilename,
    const wxString &srcMsg) {
    return generateMessage(
      pObjectFor, GOLoaderFilename::generateMessage(pFilename, srcMsg));
  }
};

#endif
