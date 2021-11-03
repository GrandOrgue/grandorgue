/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDI_H
#define GOMIDI_H

#include "GOMidiRtFactory.h"
#include "ptrvector.h"
#include <wx/event.h>

class GOMidiEvent;
class GOMidiInPort;
class GOMidiListener;
class GOMidiMap;
class GOMidiOutPort;
class GOSettings;
class GODefinitionFile;
class wxMidiEvent;

class GOMidi : public wxEvtHandler
{
private:
	GOSettings& m_Settings;
	ptr_vector<GOMidiInPort> m_midi_in_devices;
	ptr_vector<GOMidiOutPort> m_midi_out_devices;
	int m_transpose;
	std::vector<GOMidiListener*> m_Listeners;
	GOMidiRtFactory m_MidiFactory;
	void OnMidiEvent(wxMidiEvent& event);

public:

	GOMidi(GOSettings& settings);
	~GOMidi();

	void Open();
	void UpdateDevices();

	void Recv(const GOMidiEvent& e);
	void Send(const GOMidiEvent& e);

	std::vector<wxString> GetInDevices();
	std::vector<wxString> GetOutDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);
	void Register(GOMidiListener* listener);
	void Unregister(GOMidiListener* listener);

	GOMidiMap& GetMidiMap();

	DECLARE_EVENT_TABLE()
};

#endif /* GOMIDI_H */
