/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESTOP_H
#define GORGUESTOP_H

#include "GOrgueDrawStop.h"
#include <wx/string.h>
#include <vector>

class GOrgueRank;

class GOrgueStop : public GOrgueDrawstop
{
private:
	typedef struct
	{
		GOrgueRank* Rank;
		unsigned StopID;
		unsigned FirstAccessibleKeyNumber;
		unsigned FirstPipeNumber;
		unsigned PipeCount;
	} RankInfo;
	std::vector<RankInfo> m_RankInfo;
	std::vector<unsigned> m_KeyVelocity;
	unsigned m_FirstMidiNoteNumber;
	unsigned m_FirstAccessiblePipeLogicalKeyNumber;
	unsigned m_NumberOfAccessiblePipes;

	void SetRankKey(unsigned key, unsigned velocity);
	void ChangeState(bool on);
	void SetupCombinationState();

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();

public:
	GOrgueStop(GrandOrgueFile* organfile, unsigned first_midi_note_number);
	GOrgueRank* GetRank(unsigned index);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void SetKey(unsigned note, unsigned velocity);
	~GOrgueStop(void);

	unsigned IsAuto() const;

	wxString GetMidiType();
};

#endif /* GORGUESTOP_H_ */
