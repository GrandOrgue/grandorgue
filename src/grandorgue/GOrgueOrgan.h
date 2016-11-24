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

#ifndef GORGUEORGAN_H
#define GORGUEORGAN_H

#include "GOrgueMidiReceiver.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiMap;
class GOrgueSettings;

class GOrgueOrgan
{
private:
	wxString m_ODF;
	wxString m_ChurchName;
	wxString m_OrganBuilder;
	wxString m_RecordingDetail;
	wxString m_ArchiveID;
	bool m_NamesInitialized;
	long m_LastUse;
	GOrgueMidiReceiver m_midi;

public:
	GOrgueOrgan(wxString odf, wxString archive, wxString church_name, wxString organ_builder, wxString recording_detail);
	explicit GOrgueOrgan(wxString odf);
	GOrgueOrgan(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map);
	virtual ~GOrgueOrgan();

	void Update(const GOrgueOrgan& organ);

	void Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map);

	const wxString& GetODFPath() const;
	const wxString& GetChurchName() const;
	const wxString& GetOrganBuilder() const;
	const wxString& GetRecordingDetail() const;
	const wxString& GetArchiveID() const;
	const wxString GetOrganHash() const;
	long GetLastUse();
	const wxString GetUITitle();
	GOrgueMidiReceiver& GetMIDIReceiver();
	bool Match(const GOrgueMidiEvent& e);

	bool IsUsable(GOrgueSettings& settings);
};

#endif
