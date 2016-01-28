/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueOrgan.h"

#include "GOrgueArchiveFile.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueSettings.h"
#include <wx/filefn.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/stopwatch.h>

GOrgueOrgan::GOrgueOrgan(wxString odf, wxString archive, wxString church_name, wxString organ_builder, wxString recording_detail) :
	m_ODF(odf),
	m_ChurchName(church_name),
	m_OrganBuilder(organ_builder),
	m_RecordingDetail(recording_detail),
	m_ArchiveID(archive),
	m_NamesInitialized(true),
	m_midi(NULL, MIDI_RECV_ORGAN)
{
	m_LastUse = wxGetUTCTime();
}

GOrgueOrgan::GOrgueOrgan(wxString odf) :
	m_ODF(odf),
	m_ChurchName(),
	m_OrganBuilder(),
	m_RecordingDetail(),
	m_ArchiveID(),
	m_NamesInitialized(false),
	m_midi(NULL, MIDI_RECV_ORGAN)
{
	m_LastUse = wxGetUTCTime();
}

GOrgueOrgan::GOrgueOrgan(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map) :
	m_midi(NULL, MIDI_RECV_ORGAN)
{
	m_ODF = cfg.ReadString(CMBSetting, group, wxT("ODFPath"));
	m_ChurchName = cfg.ReadString(CMBSetting, group, wxT("ChurchName"));
	m_OrganBuilder = cfg.ReadString(CMBSetting, group, wxT("OrganBuilder"));
	m_RecordingDetail = cfg.ReadString(CMBSetting, group, wxT("RecordingDetail"));
	m_ArchiveID = cfg.ReadString(CMBSetting, group, wxT("Archiv"), false);
	m_LastUse = cfg.ReadInteger(CMBSetting, group, wxT("LastUse"), 0, INT_MAX, false, wxGetUTCTime());
	m_NamesInitialized = true;
	m_midi.Load(cfg, group, map);
}

GOrgueOrgan::~GOrgueOrgan()
{
}

void GOrgueOrgan::Update(const GOrgueOrgan& organ)
{
	if (m_NamesInitialized)
	{
		if (m_ChurchName != organ.m_ChurchName)
			wxLogError(_("Organ %s changed its name"), m_ChurchName.c_str());
		if (m_OrganBuilder != organ.m_OrganBuilder)
			wxLogError(_("Organ %s changed its organ builder"), m_ChurchName.c_str());
		if (m_RecordingDetail != organ.m_RecordingDetail)
			wxLogError(_("Organ %s changed its recording details"), m_ChurchName.c_str());
	}
	m_NamesInitialized = true;
	m_ChurchName = organ.m_ChurchName;
	m_OrganBuilder = organ.m_OrganBuilder;
	m_RecordingDetail = organ.m_RecordingDetail;
	m_LastUse = wxGetUTCTime();
}

const wxString& GOrgueOrgan::GetODFPath() const
{
	return m_ODF;
}

const wxString& GOrgueOrgan::GetChurchName() const
{
	return m_ChurchName;
}

const wxString& GOrgueOrgan::GetOrganBuilder() const
{
	return m_OrganBuilder;
}

const wxString& GOrgueOrgan::GetRecordingDetail() const
{
	return m_RecordingDetail;
}

const wxString& GOrgueOrgan::GetArchiveID() const
{
	return m_ArchiveID;
}

GOrgueMidiReceiver& GOrgueOrgan::GetMIDIReceiver()
{
	return m_midi;
}

const wxString GOrgueOrgan::GetUITitle()
{
	return wxString::Format(_("%s, %s"), m_ChurchName.c_str(), m_OrganBuilder.c_str());
}

long GOrgueOrgan::GetLastUse()
{
	return m_LastUse;
}

void GOrgueOrgan::Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map)
{
	cfg.WriteString(group, wxT("ODFPath"), m_ODF);
	cfg.WriteString(group, wxT("ChurchName"), m_ChurchName);
	cfg.WriteString(group, wxT("OrganBuilder"), m_OrganBuilder);
	cfg.WriteString(group, wxT("RecordingDetail"), m_RecordingDetail);
	if (m_ArchiveID != wxEmptyString)
		cfg.WriteString(group, wxT("Archiv"), m_ArchiveID);
	cfg.WriteInteger(group, wxT("LastUse"), m_LastUse);
	m_midi.Save(cfg, group, map);
}

bool GOrgueOrgan::Match(const GOrgueMidiEvent& e)
{
	switch(m_midi.Match(e))
	{
	case MIDI_MATCH_CHANGE:
	case MIDI_MATCH_ON:
		return true;

	default:
		return false;
	}
}

bool GOrgueOrgan::IsUsable(GOrgueSettings& settings)
{
	if (m_ArchiveID != wxEmptyString)
	{
		GOrgueArchiveFile* archive = settings.GetArchiveByID(m_ArchiveID, true);
		if (!archive)
			return false;
		return archive->IsComplete(settings);
	}
	else
		return wxFileExists(m_ODF);
}
