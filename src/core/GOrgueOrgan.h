/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEORGAN_H
#define GORGUEORGAN_H

#include "GOrgueMidiReceiverBase.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiMap;
class GOrgueOrganList;

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
	GOrgueMidiReceiverBase m_midi;

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
	long GetLastUse() const;
	const wxString GetUITitle() const;
	GOrgueMidiReceiverBase& GetMIDIReceiver();
	const GOrgueMidiReceiverBase& GetMIDIReceiver() const;
	bool Match(const GOrgueMidiEvent& e);

	bool IsUsable(const GOrgueOrganList& organs) const;
};

#endif
