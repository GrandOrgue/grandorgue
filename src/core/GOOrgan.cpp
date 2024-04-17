/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOOrgan::GOOrgan(
  wxString odf,
  wxString archive,
  wxString archivePath,
  wxString church_name,
  wxString organ_builder,
  wxString recording_detail)
  : m_ODF(odf),
    m_ChurchName(church_name),
    m_OrganBuilder(organ_builder),
    m_RecordingDetail(recording_detail),
    m_ArchiveID(archive),
    m_ArchivePath(archivePath),
    m_NamesInitialized(true),
    m_midi(MIDI_RECV_ORGAN) {
  m_LastUse = wxGetUTCTime();
}

GOOrgan::GOOrgan(wxString odf)
  : m_ODF(odf), m_NamesInitialized(false), m_midi(MIDI_RECV_ORGAN) {
  m_LastUse = wxGetUTCTime();
}

GOOrgan::GOOrgan(GOConfigReader &cfg, wxString group, GOMidiMap &map)
  : m_midi(MIDI_RECV_ORGAN) {
  m_ODF = cfg.ReadString(CMBSetting, group, wxT("ODFPath"));
  m_ChurchName = cfg.ReadString(CMBSetting, group, wxT("ChurchName"));
  m_OrganBuilder = cfg.ReadString(CMBSetting, group, wxT("OrganBuilder"));
  m_RecordingDetail = cfg.ReadString(CMBSetting, group, wxT("RecordingDetail"));
  m_ArchiveID = cfg.ReadString(CMBSetting, group, wxT("Archiv"), false);
  m_ArchivePath = cfg.ReadString(CMBSetting, group, wxT("ArchivePath"), false);
  m_LastUse = cfg.ReadInteger(
    CMBSetting, group, wxT("LastUse"), 0, INT_MAX, false, wxGetUTCTime());
  m_NamesInitialized = true;
  m_midi.Load(cfg, group, map);
}

GOOrgan::~GOOrgan() {}

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

GOMidiReceiverBase &GOOrgan::GetMIDIReceiver() { return m_midi; }

const wxString GOOrgan::GetUITitle() const {
  return wxString::Format(
    _("%s, %s"), m_ChurchName.c_str(), m_OrganBuilder.c_str());
}

long GOOrgan::GetLastUse() const { return m_LastUse; }

void GOOrgan::Save(GOConfigWriter &cfg, wxString group, GOMidiMap &map) {
  cfg.WriteString(group, wxT("ODFPath"), m_ODF);
  cfg.WriteString(group, wxT("ChurchName"), m_ChurchName);
  cfg.WriteString(group, wxT("OrganBuilder"), m_OrganBuilder);
  cfg.WriteString(group, wxT("RecordingDetail"), m_RecordingDetail);
  if (m_ArchiveID != wxEmptyString) {
    cfg.WriteString(group, wxT("Archiv"), m_ArchiveID);
    cfg.WriteString(group, wxT("ArchivePath"), m_ArchivePath);
  }
  cfg.WriteInteger(group, wxT("LastUse"), m_LastUse);
  m_midi.Save(cfg, group, map);
}

bool GOOrgan::Match(const GOMidiEvent &e) {
  switch (m_midi.Match(e)) {
  case MIDI_MATCH_CHANGE:
  case MIDI_MATCH_ON:
    return true;

  default:
    return false;
  }
}

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
