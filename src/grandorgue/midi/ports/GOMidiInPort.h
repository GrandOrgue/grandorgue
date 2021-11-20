/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIINPORT_H
#define GOMIDIINPORT_H

#include "ptrvector.h"

#include "GOMidiPort.h"

#include "midi/GOMidiInputMerger.h"

class GOMidiInPort: public GOMidiPort
{
protected:
	GOMidiMerger m_merger;
	int m_ChannelShift;

	virtual const wxString GetPortName() const;

	void Receive(const std::vector<unsigned char> msg);

public:
	GOMidiInPort(GOMidi* midi, wxString prefix, wxString name);
	virtual ~GOMidiInPort();

	virtual bool Open(int channel_shift = 0);
	virtual void Close() = 0;
};

#endif
