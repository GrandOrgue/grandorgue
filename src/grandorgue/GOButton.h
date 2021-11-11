/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOBUTTON_H
#define GOBUTTON_H

#include "GOEventHandler.h"
#include "GOKeyReceiver.h"
#include "GOMidiConfigurator.h"
#include "GOMidiReceiver.h"
#include "GOMidiSender.h"
#include "GOPlaybackStateHandler.h"
#include "GOSaveableObject.h"
#include <wx/string.h>

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GODefinitionFile;

class GOButton : private GOEventHandler, protected GOSaveableObject, protected GOPlaybackStateHandler,
	public GOMidiConfigurator
{
protected:
	GODefinitionFile* m_organfile;
	GOMidiReceiver m_midi;
	GOMidiSender m_sender;
	GOKeyReceiver m_shortcut;
	bool m_Pushbutton;
	bool m_Displayed;
	wxString m_Name;
	bool m_Engaged;
	bool m_DisplayInInvertedState;
	bool m_ReadOnly;

	void ProcessMidi(const GOMidiEvent& event);
	void HandleKey(int key);

	void Save(GOConfigWriter& cfg);

	void AbortPlayback();
	void StartPlayback();
	void PreparePlayback();
	void PrepareRecording();

public:
	GOButton(GODefinitionFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton);
	virtual ~GOButton();
	void Init(GOConfigReader& cfg, wxString group, wxString name);
	void Load(GOConfigReader& cfg, wxString group);
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
