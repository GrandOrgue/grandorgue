/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMANUAL_H
#define GORGUEMANUAL_H

#include "ptrvector.h"
#include "GOrgueCombinationDefinition.h"
#include "GOrgueEventHandler.h"
#include "GOrgueMidiConfigurator.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueMidiSender.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"
#include <wx/string.h>
#include <vector>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueCoupler;
class GOrgueDivisional;
class GOrgueMidiEvent;
class GOrgueStop;
class GOrgueSwitch;
class GOrgueTremulant;
class GrandOrgueFile;

class GOrgueManual : private GOrgueEventHandler, private GOrgueSaveableObject, private GOrguePlaybackStateHandler,
	public GOrgueMidiConfigurator
{
private:
	wxString m_group;
	GOrgueMidiReceiver m_midi;
	GOrgueMidiSender m_sender;
	GOrgueMidiSender m_division;
	GrandOrgueFile* m_organfile;
	std::vector<GOrgueCoupler*> m_InputCouplers;
	/* Keyboard state */
	std::vector<unsigned> m_KeyVelocity;
	/* Internal state affected by couplers */
	std::vector<unsigned> m_RemoteVelocity;
	std::vector<unsigned> m_Velocity;
	std::vector<unsigned> m_DivisionState;
	std::vector<std::vector<unsigned> > m_Velocities;
	unsigned m_MidiMap[128];
	unsigned m_manual_number;
	unsigned m_first_accessible_logical_key_nb;
	unsigned m_nb_logical_keys;
	unsigned m_first_accessible_key_midi_note_nb;
	unsigned m_nb_accessible_keys;
	unsigned m_UnisonOff;

	int m_MIDIInputNumber;

	std::vector<unsigned> m_tremulant_ids;
	std::vector<unsigned> m_switch_ids;

	wxString m_name;

	ptr_vector<GOrgueStop> m_stops;
	ptr_vector<GOrgueCoupler> m_couplers;
	ptr_vector<GOrgueDivisional> m_divisionals;
	unsigned m_ODFCouplerCount;
	bool m_displayed;
	GOrgueCombinationDefinition m_DivisionalTemplate;

	void Resize();

	void ProcessMidi(const GOrgueMidiEvent& event);
	void HandleKey(int key);
	void SetOutput(unsigned note, unsigned velocity);

	void Save(GOrgueConfigWriter& cfg);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:

	GOrgueManual(GrandOrgueFile* organfile);
	void Init(GOrgueConfigReader& cfg, wxString group, int manualNumber, unsigned first_midi, unsigned keys);
	void Load(GOrgueConfigReader& cfg, wxString group, int manualNumber);
	unsigned RegisterCoupler(GOrgueCoupler* coupler);
	void SetKey(unsigned note, unsigned velocity, GOrgueCoupler* prev, unsigned couplerID);
	void Set(unsigned note, unsigned velocity);
	void SetUnisonOff(bool on);
	void Update();
	void Reset();
	void SetElementID(int id);
	~GOrgueManual(void);

	unsigned GetNumberOfAccessibleKeys();
	unsigned GetFirstAccessibleKeyMIDINoteNumber();
	int GetFirstLogicalKeyMIDINoteNumber();
	int GetMIDIInputNumber();
	unsigned GetLogicalKeyCount();
	void AllNotesOff();
	bool IsKeyDown(unsigned midiNoteNumber);

	unsigned GetStopCount();
	GOrgueStop* GetStop(unsigned index);
	unsigned GetCouplerCount();
	unsigned GetODFCouplerCount();
	GOrgueCoupler* GetCoupler(unsigned index);
	void AddCoupler(GOrgueCoupler* coupler);
	unsigned GetDivisionalCount();
	GOrgueDivisional* GetDivisional(unsigned index);
 	void AddDivisional(GOrgueDivisional* divisional);
	unsigned GetTremulantCount();
	GOrgueTremulant* GetTremulant(unsigned index);
	unsigned GetSwitchCount();
	GOrgueSwitch* GetSwitch(unsigned index);

	GOrgueCombinationDefinition& GetDivisionalTemplate();
	const wxString& GetName();
	bool IsDisplayed();

	wxString GetMidiType();
	wxString GetMidiName();
	void ShowConfigDialog();

	wxString GetElementStatus();
	std::vector<wxString> GetElementActions();
	void TriggerElementActions(unsigned no);
};

#endif
