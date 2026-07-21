/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGAN_H
#define GOORGAN_H

#include <wx/string.h>

class GOOrganList;

class GOOrgan {
protected:
  wxString m_ODF;
  wxString m_ChurchName;
  wxString m_OrganBuilder;
  wxString m_RecordingDetail;
  wxString m_ArchiveID;
  wxString m_ArchivePath;
  bool m_NamesInitialized;
  long m_LastUse;

public:
  GOOrgan(
    const wxString &odf,
    const wxString &archive,
    const wxString &archivePath,
    const wxString &church_name,
    const wxString &organ_builder,
    const wxString &recording_detail);
  explicit GOOrgan(wxString odf);
  virtual ~GOOrgan() {}

  void Update(const GOOrgan &organ);

  const wxString &GetODFPath() const;
  /**
   * Sets the path of the ODF file. If an archive is set (GetArchiveID() is
   * not empty), the path is relative to that archive and is stored as-is;
   * otherwise it is a normal filesystem path and is normalized to an
   * absolute path.
   * @param newPath - the archive-relative or filesystem ODF path
   */
  void SetODFPath(const wxString &newPath);
  const wxString &GetChurchName() const;
  const wxString &GetOrganBuilder() const;
  const wxString &GetRecordingDetail() const;
  const wxString &GetArchiveID() const { return m_ArchiveID; }
  const wxString &GetArchivePath() const { return m_ArchivePath; }
  void SetArchivePath(const wxString &archivePath) {
    m_ArchivePath = archivePath;
  }
  const wxString &GetPath() const {
    return m_ArchiveID.IsEmpty() ? m_ODF : m_ArchivePath;
  }
  const wxString GetOrganHash() const;
  long GetLastUse() const;
  const wxString GetUITitle() const;

  bool IsUsable(const GOOrganList &organs) const;
};

#endif
