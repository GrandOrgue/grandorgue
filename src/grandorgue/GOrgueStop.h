/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESTOP_H
#define GORGUESTOP_H

#include <vector>
#include <wx/wx.h>
#include "GOrgueDrawStop.h"

class GOrgueRank;

class GOrgueStop : public GOrgueDrawstop
{
private:
	typedef struct
	{
		GOrgueRank* Rank;
		unsigned FirstAccessibleKeyNumber;
		unsigned FirstPipeNumber;
		unsigned PipeCount;
	} RankInfo;
	std::vector<RankInfo> m_RankInfo;
	std::vector<unsigned> m_KeyState;
	unsigned m_ManualNumber;
	unsigned m_FirstMidiNoteNumber;
	unsigned m_FirstAccessiblePipeLogicalKeyNumber;
	unsigned m_NumberOfAccessiblePipes;

	void SetRankKey(unsigned key, bool on);

public:
	GOrgueStop(GrandOrgueFile* organfile, unsigned manual_number, unsigned first_midi_note_number);
	GOrgueRank* GetRank(unsigned index);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Set(bool on);
	void SetKey(unsigned note, int on);
	void Abort();
	void PreparePlayback();
	~GOrgueStop(void);

	unsigned IsAuto() const;
};

#endif /* GORGUESTOP_H_ */
