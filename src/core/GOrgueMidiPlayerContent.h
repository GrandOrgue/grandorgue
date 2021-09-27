/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIPLAYERCONTENT_H
#define GORGUEMIDIPLAYERCONTENT_H

#include <wx/string.h>
#include <vector>

class GOrgueMidiEvent;
class GOrgueMidiMap;
class GOrgueMidiFileReader;

class GOrgueMidiPlayerContent
{
private:
	std::vector<GOrgueMidiEvent> m_Events;
	unsigned m_Pos;

	void ReadFileContent(GOrgueMidiFileReader& reader, std::vector<GOrgueMidiEvent>& events);
	void SetupManual(GOrgueMidiMap& map, unsigned channel, wxString ID);

public:
	GOrgueMidiPlayerContent();
	virtual ~GOrgueMidiPlayerContent();

	void Clear();
	void Reset();
	bool IsLoaded();
	bool Load(GOrgueMidiFileReader& reader, GOrgueMidiMap& map, unsigned manuals, bool pedal);

	const GOrgueMidiEvent& GetCurrentEvent();
	bool Next();
};

#endif
