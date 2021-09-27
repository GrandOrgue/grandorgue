/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMETRONOME_H
#define GORGUEMETRONOME_H

#include "GOrgueElementCreator.h"
#include "GOrgueLabel.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"
#include "GOrgueTimerCallback.h"

class GOrgueMidiEvent;
class GOrgueRank;
class GrandOrgueFile;

class GOrgueMetronome : private GOrgueTimerCallback, private GOrguePlaybackStateHandler,
	private GOrgueSaveableObject, public GOrgueElementCreator
{
private:
	GrandOrgueFile* m_organfile;
	unsigned m_BPM;
	unsigned m_MeasureLength;
	unsigned m_Pos;
	bool m_Running;
	GOrgueLabel m_BPMDisplay;
	GOrgueLabel m_MeasureDisplay;
	GOrgueRank* m_rank;
	unsigned m_StopID;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void HandleTimer();

	void ButtonChanged(int id);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

	void Save(GOrgueConfigWriter& cfg);

	void StartTimer();
	void StopTimer();
	void UpdateState();
	void UpdateBPM(int val);
	void UpdateMeasure(int val);

public:
	GOrgueMetronome(GrandOrgueFile* organfile);
	virtual ~GOrgueMetronome();

	void Load(GOrgueConfigReader& cfg);

	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
