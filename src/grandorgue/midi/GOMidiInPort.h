/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIINPORT_H
#define GOMIDIINPORT_H

#include "midi/GOMidiMerger.h"
#include "ptrvector.h"
#include <wx/string.h>

class GOMidi;

class GOMidiInPort
{
protected:
	GOMidi* m_midi;
	bool m_IsActive;
	wxString m_Name;
	wxString m_Prefix;
	unsigned m_ID;
	GOMidiMerger m_merger;
	int m_ChannelShift;

	const wxString GetClientName();
	const wxString GetPortName();
	void Receive(const std::vector<unsigned char> msg);

public:
	GOMidiInPort(GOMidi* midi, wxString prefix, wxString name);
	virtual ~GOMidiInPort();

	virtual bool Open(int channel_shift = 0);
	virtual void Close() = 0;

	const wxString& GetName();
	unsigned GetID();
	bool IsActive();
};

#endif
