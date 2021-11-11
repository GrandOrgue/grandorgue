/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOORGAN_H
#define GOORGAN_H

#include "midi/GOMidiReceiverBase.h"
#include <wx/string.h>

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GOOrganList;

class GOOrgan
{
private:
	wxString m_ODF;
	wxString m_ChurchName;
	wxString m_OrganBuilder;
	wxString m_RecordingDetail;
	wxString m_ArchiveID;
	bool m_NamesInitialized;
	long m_LastUse;
	GOMidiReceiverBase m_midi;

public:
	GOOrgan(wxString odf, wxString archive, wxString church_name, wxString organ_builder, wxString recording_detail);
	explicit GOOrgan(wxString odf);
	GOOrgan(GOConfigReader& cfg, wxString group, GOMidiMap& map);
	virtual ~GOOrgan();

	void Update(const GOOrgan& organ);

	void Save(GOConfigWriter& cfg, wxString group, GOMidiMap& map);

	const wxString& GetODFPath() const;
	const wxString& GetChurchName() const;
	const wxString& GetOrganBuilder() const;
	const wxString& GetRecordingDetail() const;
	const wxString& GetArchiveID() const;
	const wxString GetOrganHash() const;
	long GetLastUse() const;
	const wxString GetUITitle() const;
	GOMidiReceiverBase& GetMIDIReceiver();
	const GOMidiReceiverBase& GetMIDIReceiver() const;
	bool Match(const GOMidiEvent& e);

	bool IsUsable(const GOOrganList& organs) const;
};

#endif
