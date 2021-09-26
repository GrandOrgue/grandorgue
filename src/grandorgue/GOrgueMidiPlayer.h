/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIPLAYER_H
#define GORGUEMIDIPLAYER_H

#include "GOrgueLabel.h"
#include "GOrgueElementCreator.h"
#include "GOrgueMidiPlayerContent.h"
#include "GOrgueTime.h"
#include "GOrgueTimerCallback.h"
#include <wx/string.h>
#include <wx/timer.h>
#include <vector>

class GOrgueMidiEvent;
class GOrgueMidiFileReader;
class GrandOrgueFile;

class GOrgueMidiPlayer : public GOrgueElementCreator, private GOrgueTimerCallback
{
private:
	GrandOrgueFile* m_organfile;
	GOrgueMidiPlayerContent m_content;
	GOrgueLabel m_PlayingTime;
	GOTime m_Start;
	unsigned m_PlayingSeconds;
	float m_Speed;
	bool m_IsPlaying;
	bool m_Pause;
	unsigned m_DeviceID;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void ButtonChanged(int id);

	void UpdateDisplay();
	void HandleTimer();

public:
	GOrgueMidiPlayer(GrandOrgueFile* organfile);
	~GOrgueMidiPlayer();

	void Clear();
	void LoadFile(const wxString& filename, unsigned manuals, bool pedal);
	bool IsLoaded();

	void Play();
	void Pause();
	void StopPlaying();
	bool IsPlaying();

	void Load(GOrgueConfigReader& cfg);
	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
