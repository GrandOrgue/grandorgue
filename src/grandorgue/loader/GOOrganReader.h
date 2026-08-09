/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANREADER_H
#define GOORGANREADER_H

#include <wx/string.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"

#include "GOLoadedOrganInfo.h"

class GOConfig;
class GOFileStore;
class GOOrgan;
class GOProgressMonitor;

/**
 * Resolves an organ's archive/ODF/CMB files, reads them into a config
 * database, and reports the paths and hash discovered along the way. Owns
 * the GOConfigReaderDB/GOConfigReader pair for the duration of one load, so
 * the caller must keep the GOOrganReader instance alive while reading from
 * GetConfigReader().
 */
class GOOrganReader {
private:
  wxString m_Hash;
  GOConfigReaderDB m_ConfigDB;
  GOConfigReader m_ConfigReader;
  GOLoadedOrganInfo m_LoadedOrganInfo;

public:
  /**
   * Reads the ODF and its matching CMB (explicit `settingsPath`, the
   * previously saved per-user CMB if it exists, or a bundled/embedded
   * CMB) for `organ`, populating the config DB and the result returned by
   * GetLoadedOrganInfo(). Throws a wxString error message on failure.
   * @param config - the application config, used for ODF strictness checks
   *   and settings/cache path generation. Not const because it is passed on
   *   to GOFileStore::LoadArchives(GOOrganList&, ...), which takes its
   *   GOOrganList base (config's archive list) by non-const reference even
   *   though it only reads from it
   * @param organ - the organ to load, giving the (self-normalized) ODF
   *   path and/or archive info
   * @param settingsPath - an explicit CMB path to import, or empty to
   *   auto-discover (previously saved per-user CMB, then bundled/embedded
   *   CMB)
   * @param fileStore - the file store to resolve archives and read the
   *   ODF/CMB from
   * @param monitor - receives the same progress messages the code this
   *   replaces reported
   */
  GOOrganReader(
    GOConfig &config,
    const GOOrgan &organ,
    const wxString &settingsPath,
    GOFileStore &fileStore,
    GOProgressMonitor &monitor);

  /**
   * @return the config database populated by the constructor, shared with
   *   GetConfigReader()
   */
  GOConfigReaderDB &GetConfigDB() { return m_ConfigDB; }
  /**
   * @return the config reader wrapping GetConfigDB(), for reading ODF/CMB
   *   settings
   */
  GOConfigReader &GetConfigReader() { return m_ConfigReader; }
  /**
   * @return the paths, hash, and customization flag discovered while
   *   reading the organ
   */
  const GOLoadedOrganInfo &GetLoadedOrganInfo() const {
    return m_LoadedOrganInfo;
  }
  /** Logs a warning for every ODF/CMB entry that was never read. */
  void ReportUnused() { m_ConfigDB.ReportUnused(); }
};

#endif /* GOORGANREADER_H */
