/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrgan.h"

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/stopwatch.h>

#include "GOHash.h"
#include "GOOrganList.h"
#include "archive/GOArchiveFile.h"

GOOrgan::GOOrgan(
  const wxString &odf,
  const wxString &archive,
  const wxString &archivePath,
  const wxString &church_name,
  const wxString &organ_builder,
  const wxString &recording_detail)
  : m_ODF(odf),
    m_ChurchName(church_name),
    m_OrganBuilder(organ_builder),
    m_RecordingDetail(recording_detail),
    m_ArchiveID(archive),
    m_ArchivePath(archivePath),
    m_NamesInitialized(true) {
  m_LastUse = wxGetUTCTime();
}

GOOrgan::GOOrgan(wxString odf) : m_ODF(odf), m_NamesInitialized(false) {
  m_LastUse = wxGetUTCTime();
}

void GOOrgan::Update(const GOOrgan &organ) {
  if (m_NamesInitialized) {
    if (m_ChurchName != organ.m_ChurchName)
      wxLogError(_("Organ %s changed its name"), m_ChurchName.c_str());
    if (m_OrganBuilder != organ.m_OrganBuilder)
      wxLogError(_("Organ %s changed its organ builder"), m_ChurchName.c_str());
    if (m_RecordingDetail != organ.m_RecordingDetail)
      wxLogError(
        _("Organ %s changed its recording details"), m_ChurchName.c_str());
  }
  m_NamesInitialized = true;
  m_ChurchName = organ.m_ChurchName;
  m_OrganBuilder = organ.m_OrganBuilder;
  m_RecordingDetail = organ.m_RecordingDetail;
  m_LastUse = wxGetUTCTime();
}

const wxString &GOOrgan::GetODFPath() const { return m_ODF; }

const wxString &GOOrgan::GetChurchName() const { return m_ChurchName; }

const wxString &GOOrgan::GetOrganBuilder() const { return m_OrganBuilder; }

const wxString &GOOrgan::GetRecordingDetail() const {
  return m_RecordingDetail;
}

const wxString GOOrgan::GetUITitle() const {
  return wxString::Format(
    _("%s, %s"), m_ChurchName.c_str(), m_OrganBuilder.c_str());
}

long GOOrgan::GetLastUse() const { return m_LastUse; }

bool GOOrgan::IsUsable(const GOOrganList &organs) const {
  bool res;

  if (!m_ArchiveID.IsEmpty()) {
    const GOArchiveFile *archive = m_ArchivePath.IsEmpty()
      ? organs.GetArchiveByID(m_ArchiveID, true)
      : organs.GetArchiveByPath(m_ArchivePath);

    res = archive && archive->IsComplete(organs);
  } else
    res = wxFileExists(m_ODF);
  return res;
}

const wxString GOOrgan::GetOrganHash() const {
  GOHash hash;

  if (m_ArchiveID != wxEmptyString) {
    hash.Update(m_ArchiveID);
    hash.Update(m_ODF);
  } else {
    wxFileName odf(m_ODF);

    odf.Normalize(
      wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_CASE
      | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);
    wxString filename = odf.GetFullPath();

    hash.Update(filename);
  }

  return hash.getStringHash();
}
