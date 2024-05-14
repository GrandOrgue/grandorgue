/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADERFILENAME_H
#define GOLOADERFILENAME_H

#include <memory>

#include <wx/string.h>

class GOOpenedFile;
class GOFileStore;
class GOHash;

/**
 * This class represent a referrence to a file in a virtual filesystem
 * There are three virtual filesystem roots available:
 *   - ODF: the file in the directory near the ODF file or in an
 *     archive (in the main one or a referenced one). Usually for refs from ODF
 *   - Resource: the file is somewhere in the resource directory near the
 *     GrandOrgue executable. Usually for refs from GrandOrgue itself
 *   - Absolute: the file is somewhere in the host filesystem. Usually for refs
 *     from the config.
 * The name consist of two parts: the root and the path inside the root.
 * The file is actually searched during the Open().
 */

class GOLoaderFilename {
private:
  enum RootKind { ROOT_UNKNOWN, ROOT_ODF, ROOT_RESOURCE, ROOT_ABSOLUTE };

  RootKind m_RootKind = ROOT_UNKNOWN;
  wxString m_path; // relative the root

  void Assign(const RootKind rootKind, const wxString &path);

public:
  void Assign(const wxString &path) { Assign(ROOT_ODF, path); }
  void AssignResource(const wxString &path) { Assign(ROOT_RESOURCE, path); }
  void AssignAbsolute(const wxString &path) { Assign(ROOT_ABSOLUTE, path); }

  const wxString &GetPath() const { return m_path; }
  void Hash(GOHash &hash) const;

  /**
   * Opens Searches the file and opens it. If the file does not exist then
   *   throws an exception
   * @param fileStore a GOFileStore object for searching the file against
   * @return a pointer to the GOFile
   */
  std::unique_ptr<GOOpenedFile> Open(const GOFileStore &fileStore) const;

  wxString GenerateMessage(const wxString &srcMsg) const {
    return wxString::Format("%s: %s", m_path, srcMsg);
  }

  static wxString generateMessage(
    const GOLoaderFilename *pFileName, const wxString &srcMsg) {
    return pFileName ? pFileName->GenerateMessage(srcMsg) : srcMsg;
  }
};

#endif
