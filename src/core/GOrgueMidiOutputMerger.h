/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIOUTPUTMERGER_H
#define GORGUEMIDIOUTPUTMERGER_H

#include "GOrgueMidiEvent.h"
#include <wx/string.h>
#include <vector>

class GOrgueMidiOutputEvent;

class GOrgueMidiOutputMerger
{
private:
	typedef struct {
		midi_message_type type;
		int key;
		wxString content;
	} GOrgueMidiOutputMergerHWState;
	std::vector<GOrgueMidiOutputMergerHWState> m_HWState;
	std::vector<std::vector<uint8_t> > m_RodgersState;

public:
	GOrgueMidiOutputMerger();

	void Clear();
	bool Process(GOrgueMidiEvent& e);
};

#endif
