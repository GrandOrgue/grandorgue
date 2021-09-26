/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIFILEREADER_H
#define GORGUEMIDIFILEREADER_H

#include "GOrgueBuffer.h"
#include <wx/string.h>
#include <stdint.h>

class GOrgueMidiEvent;
class GOrgueMidiMap;

class GOrgueMidiFileReader
{
private:
	GOrgueMidiMap& m_Map;
	GOrgueBuffer<uint8_t> m_Data;
	unsigned m_Tracks;
	float m_Speed;
	unsigned m_Pos;
	unsigned m_TrackEnd;
	unsigned m_LastStatus;
	unsigned m_CurrentSpeed;
	float m_LastTime;
	unsigned m_PPQ;
	unsigned m_Tempo;

	bool StartTrack();
	unsigned DecodeTime();

public:
	GOrgueMidiFileReader(GOrgueMidiMap& map);
	~GOrgueMidiFileReader();

	bool Open(wxString filename);
	bool ReadEvent(GOrgueMidiEvent& e);
	bool Close();
};

#endif
