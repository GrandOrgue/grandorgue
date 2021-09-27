/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEBUTTON_H
#define GORGUEBUTTON_H

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

class GOrgueButton : private GOrgueEventHandler, protected GOrgueSaveableObject, protected GOrguePlaybackStateHandler,
	public GOrgueMidiConfigurator
{
protected:
	GrandOrgueFile* m_organfile;
	GOrgueMidiReceiver m_midi;
	GOrgueMidiSender m_sender;
	GOrgueKeyReceiver m_shortcut;
	bool m_Pushbutton;
	bool m_Displayed;
	wxString m_Name;
	bool m_Engaged;
	bool m_DisplayInInvertedState;
	bool m_ReadOnly;

	void ProcessMidi(const GOrgueMidiEvent& event);
	void HandleKey(int key);

	void Save(GOrgueConfigWriter& cfg);

	void AbortPlayback();
	void StartPlayback();
	void PreparePlayback();
	void PrepareRecording();

public:
	GOrgueButton(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton);
	virtual ~GOrgueButton();
	void Init(GOrgueConfigReader& cfg, wxString group, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group);
	bool IsDisplayed();
	bool IsReadOnly();
	const wxString& GetName();

	virtual void Push();
	virtual void Set(bool on);
	virtual void Display(bool onoff);
	bool IsEngaged() const;
	bool DisplayInverted() const;
	void SetElementID(int id);
	void SetShortcutKey(unsigned key);
	void SetPreconfigIndex(unsigned index);

	wxString GetMidiName();
	void ShowConfigDialog();

	wxString GetElementStatus();
	std::vector<wxString> GetElementActions();
	void TriggerElementActions(unsigned no);
};

#endif
