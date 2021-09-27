/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIOUTPORT_H
#define GORGUEMIDIOUTPORT_H

#include "GOrgueMidiOutputMerger.h"
#include <wx/string.h>
#include "ptrvector.h"

class GOrgueMidi;
class GOrgueMidiEvent;

class GOrgueMidiOutPort
{
protected:
	GOrgueMidi* m_midi;
	GOrgueMidiOutputMerger m_merger;
	bool m_IsActive;
	wxString m_Name;
	wxString m_Prefix;
	unsigned m_ID;

	const wxString GetClientName();
	const wxString GetPortName();
	virtual void SendData(std::vector<unsigned char>& msg) = 0;

public:
	GOrgueMidiOutPort(GOrgueMidi* midi, wxString prefix, wxString name);
	virtual ~GOrgueMidiOutPort();

	virtual bool Open();
	virtual void Close() = 0;

	void Send(const GOrgueMidiEvent& e);

	const wxString& GetName();
	unsigned GetID();
	bool IsActive();
};

#endif
