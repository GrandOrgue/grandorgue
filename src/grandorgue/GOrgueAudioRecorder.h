/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEAUDIORECORDER_H
#define GORGUEAUDIORECORDER_H

#include "GOrgueElementCreator.h"
#include "GOrgueLabel.h"
#include "GOrgueTimerCallback.h"
#include <wx/string.h>

class GrandOrgueFile;
class GOSoundRecorder;

class GOrgueAudioRecorder : public GOrgueElementCreator, private GOrgueTimerCallback
{
private:
	GrandOrgueFile* m_organfile;
	GOSoundRecorder* m_recorder;
	GOrgueLabel m_RecordingTime;
	unsigned m_RecordSeconds;
	wxString m_Filename;
	bool m_DoRename;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void ButtonChanged(int id);

	void UpdateDisplay();
	void HandleTimer();

public:
	GOrgueAudioRecorder(GrandOrgueFile* organfile);
	~GOrgueAudioRecorder();

	void SetAudioRecorder(GOSoundRecorder* recorder);

	void StartRecording(bool rename);
	bool IsRecording();
	void StopRecording();

	void Load(GOrgueConfigReader& cfg);
	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
