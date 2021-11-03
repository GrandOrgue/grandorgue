/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIOUTPORT_H
#define GOMIDIOUTPORT_H

#include "midi/GOMidiOutputMerger.h"
#include <wx/string.h>
#include "ptrvector.h"

class GOMidi;
class GOMidiEvent;

class GOMidiOutPort
{
protected:
	GOMidi* m_midi;
	GOMidiOutputMerger m_merger;
	bool m_IsActive;
	wxString m_Name;
	wxString m_Prefix;
	unsigned m_ID;

	const wxString GetClientName();
	const wxString GetPortName();
	virtual void SendData(std::vector<unsigned char>& msg) = 0;

public:
	GOMidiOutPort(GOMidi* midi, wxString prefix, wxString name);
	virtual ~GOMidiOutPort();

	virtual bool Open();
	virtual void Close() = 0;

	void Send(const GOMidiEvent& e);

	const wxString& GetName();
	unsigned GetID();
	bool IsActive();
};

#endif
