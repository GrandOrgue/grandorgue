/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEENCLOSURE_H_
#define GORGUEENCLOSURE_H_

#include "GOrgueEventHandler.h"
#include "GOrgueKeyReceiver.h"
#include "GOrgueMidiConfigurator.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueMidiSender.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiEvent;
class GrandOrgueFile;

class GOrgueEnclosure : private GOrgueEventHandler, private GOrgueSaveableObject, private GOrguePlaybackStateHandler,
	public GOrgueMidiConfigurator
{
private:
	GOrgueMidiReceiver m_midi;
	GOrgueMidiSender m_sender;
	GOrgueKeyReceiver m_shortcut;
	GrandOrgueFile* m_organfile;
	int m_AmpMinimumLevel;
	int m_MIDIInputNumber;
	int m_MIDIValue;
	wxString m_Name;
	bool m_Displayed1;
	bool m_Displayed2;

	void ProcessMidi(const GOrgueMidiEvent& event);
	void HandleKey(int key);

	void Save(GOrgueConfigWriter& cfg);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:

	GOrgueEnclosure(GrandOrgueFile* organfile);
	virtual ~GOrgueEnclosure();
	void Init(GOrgueConfigReader& cfg, wxString group, wxString Name, unsigned def_value);
	void Load(GOrgueConfigReader& cfg, wxString group, int enclosure_nb);
	void Set(int n);
	const wxString& GetName();
	int GetValue();
	int GetMIDIInputNumber();
	float GetAttenuation();

	void Scroll(bool scroll_up);
	bool IsDisplayed(bool new_format);
	void SetElementID(int id);

	wxString GetMidiType();
	wxString GetMidiName();
	void ShowConfigDialog();

	wxString GetElementStatus();
	std::vector<wxString> GetElementActions();
	void TriggerElementActions(unsigned no);
};

#endif /* GORGUEENCLOSURE_H_ */
